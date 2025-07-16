#include "parser.h"
#include "cppstate.h"

#include "parsing/syntax.h"
#include "tools/commandline.h"
#include "tools/log.h"
#include "tools/tools.h"

#include "project/command.h"
#include "project/targetid.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr auto Dot = ".";
}

Parser::Parser(const CommandLine *cmd) : _input(cmd->input()), _cmd(cmd)
{
    if (_input.empty())
    {
        _input = std::filesystem::current_path();
    }

    if (not std::filesystem::exists(_input))
    {
        Log::error("Input path does not exist, cannot continue:", _input);
        _status = AppError::WrongInputPath;
        return;
    }

    const auto dir = std::filesystem::directory_entry(_input);

    if (dir.is_regular_file())
    {
        Log::information("Got a regular file ", _input);
        _projectEntryPoint = dir;
        const auto base = std::filesystem::path(_input).remove_filename();
        _projectDirectory = std::filesystem::directory_entry(base);

        const auto type = fileType(_input);

        if (type == Syntax::FileType::Project)
        {
            _projectFile = dir;
        }
        else if (type == Syntax::FileType::Cpp)
        {
            _projectEntryPoint = dir;
        }
        else
        {
            Log::error(
                "Input file type is incorrect: neither .gibs, nor a C++ source file:",
                _input, "Extension is:", _input.extension());
            _status = AppError::IncorrectInputFileType;
            return;
        }
    }
    else if (dir.is_directory())
    {
        Log::information("Got a directory, will scan it for project files or main.cpp:",
                         _input);
        _projectDirectory = dir;

        scanProjectDirectoryForEntryPoints();
    }

    addIncludePath(_projectDirectory);
}

AppError Parser::status() const
{
    return AppError();
}

void Parser::parse()
{
    // Take project name from parent directory - for now. It can be adjusted later if
    // "target name" command is found inside project files
    _project.id =
        TargetId(_projectDirectory.parent_path().filename(), TargetId::Type::Executable);

    Log::information("Project name:", _project.id.name());

    if (_projectFile.has_filename())
    {
        parseProjectFile(_projectFile, _project.id);
    }

    if (_projectEntryPoint.has_filename())
    {
        Command link;
        link.targetId = _project.id;
        // TODO: executable or library or just target - decide
        link.type = Syntax::Command::Executable;
        link.append(_project.id.name());
        link.finalize();
        _project.addCommand(link);
        parseCppFile(_projectEntryPoint, _project.id);
    }

    _project.logCommandTree();
}

bool Parser::scanProjectDirectoryForEntryPoints()
{
    const auto &dir = _projectDirectory;

    for (auto const &it : std::filesystem::directory_iterator(dir))
    {
        const auto type = fileType(it.path());

        if (type == Syntax::FileType::Project)
        {
            _projectFile = it.path();
        }
        else if (type == Syntax::FileType::Cpp)
        {
            if (it.path().filename() == Syntax::Extension::Main)
            {
                _projectEntryPoint = it.path();
            }
        }
    }

    if (std::filesystem::directory_entry(_projectFile).exists() or
        std::filesystem::directory_entry(_projectEntryPoint).exists())
    {
        return true;
    }

    Log::error("Neither .gibs, nor a C++ source file has been found in directory:",
               _projectDirectory);

    _status = AppError::EntryPointNotFound;
    return false;
}

void Parser::parseProjectFile(const std::filesystem::path &path, const TargetId &id)
{
    std::ifstream file(path, std::iostream::in);

    if (file.is_open() == false)
    {
        Log::error("Could not open file for reading:", path);
        return;
    }

    Log::debug("Reading project file:", path);

    std::string line;
    // TODO: implement a custom file reading routine to read it character by character and
    // parse on the fly
    while (std::getline(file, line))
    {
        Log::verbose("Read:", path.filename(), ":", line);
        parseProjectLine(std::move(line), id);
    }

    file.close();
}

void Parser::parseCppFile(const std::filesystem::path &path, const TargetId &id)
{
    if (Tools::contains(_compiledFiles, path.string()))
    {
        Log::verbose("Skipping, already parsed:", path);
        return;
    }

    addIncludePath(path);

    std::ifstream file(path, std::iostream::in);

    if (not file.is_open())
    {
        Log::error("Could not open file for reading:", path);
        return;
    }

    Log::debug("Reading file:", path);
    _compiledFiles.push_back(path);

    CppState state;
    state.id = id;

    std::string line;

    // TODO: implement a custom file reading routine to read it character by character and
    // parse on the fly
    while (std::getline(file, line))
    {
        Log::verbose("Read:", path.filename(), ":", line);
        parseCppLine(std::move(line), &state);

        if (state.shouldFinish)
        {
            break;
        }
    }

    file.close();

    const auto type = fileType(path);

    if (type == Syntax::FileType::Cpp)
    {
        // Prepare link command if not already present:
        auto linkId = _project.linkCommandIdFor(state.id);
        if (linkId == 0) [[unlikely]]
        {
            Command link;
            link.type = state.id.type() == TargetId::Type::Executable
                            ? Syntax::Command::Executable
                            : Syntax::Command::Library;
            link.targetId = id;
            link.append(std::filesystem::relative(state.id.name(), root()));
            link.finalize();
            _project.addCommand(link);
        }

        auto linkCommand = &_project.commandRef(linkId);

        // Now, add compilation command for this cpp file:
        Command compile;
        compile.type = Syntax::Command::Source;
        compile.append(std::filesystem::relative(path, root()));
        compile.targetId = state.id;
        compile.parentId = linkCommand->id();
        compile.finalize();

        Log::debug("Adding object file to linker command:", compile.object().name);

        linkCommand->addLinkObject(compile.object().name);

        Log::information("Compiling cpp file:", path.filename());
        _project.addCommand(compile);
        _processor.schedule(compile);

        // TODO: only execute this command after all children have finished processing!
        //_processor.schedule(link);
    }
    else if (type == Syntax::FileType::H)
    {
        Log::debug("Looking for a source file accompanying this header:",
                   path.filename());
        // TODO: handle case where source file is in a different directory... maybe cache
        // the dir structure ?
        // }
        const auto cppPathOptional = findCppFile(path.filename());

        if (cppPathOptional.has_value()) [[likely]]
        {
            const auto &cppPath = cppPathOptional.value();
            if (fileType(cppPath) == Syntax::FileType::Cpp)
            {
                Log::information("Parsing cpp file for header:", path.filename(),
                                 "under target ID:", state.id);
                parseCppFile(cppPath, state.id);
            }
        }
        else [[unlikely]]
        {
            Log::debug("Not found!");
        }
    }
}

void Parser::parseProjectLine(std::string &&line, const TargetId &id)
{
    if (line.size() == 0)
    {
        return;
    }

    if (line.at(0) == Syntax::Comment::Project)
    {
        Log::debug("Found a comment, ignoring...");
        return;
    }

    std::string word;
    Command command;

    for (const auto &character : std::as_const(line))
    {
        if (Tools::isWhitespace(character))
        {
            if (word.empty() == true)
            {
                // Skip indentation and long whitespace
                continue;
            }

            // Make sure word gets cleaned up even if we exit early
            const auto guard = Tools::ScopeGuard([&word] { word.clear(); });

            if (word.size() == 1 and word.at(0) == Syntax::Comment::Project) [[unlikely]]
            {
                // Skip comment line
                continue;
            }

            command.append(word);
        }
    }

    command.finalize();

    CppState state;
    state.id = id;
    handleCommand(command, &state);
}

void Parser::parseCppLine(std::string &&line, CppState *state)
{
    assert(state);

    if (line.size() == 0)
    {
        return;
    }

    std::string word;
    Command command;
    bool isOneLineCommand = false;

    enum class Action
    {
        Continue,
        Break
    };

    const auto processWord = [&](std::string &word, auto &state) -> Action
    {
        // Make sure word gets cleaned up even if we exit early
        const auto guard = Tools::ScopeGuard([&word] { word.clear(); });

        if (word == Syntax::Comment::MultilineEnd) [[unlikely]]
        {
            state->isCommentBlock = false;
            state->isProjectCommentBlock = false;
            return Action::Continue;
        }

        // TODO: when a comment begins mid-line, finish existing Command

        // TODO: update TargetId properly!

        // Recognize project comments and comment blocks:
        if (word == Syntax::Comment::MultilineBeginProject)
        {
            state->isProjectCommentBlock = true;
            return Action::Continue;
        }
        else if (word == Syntax::Comment::OneLine)
        {
            return Action::Continue;
        }
        else if (word == Syntax::Comment::MultilineBegin) [[unlikely]]
        {
            // Recognize C++ comments and comment blocks:
            state->isCommentBlock = true;
            return Action::Continue;
        }

        if (state->isCommentBlock)
        {
            // Skip C++ comments
            return Action::Continue;
        }

        if (word == Syntax::CppKeywords::Include)
        {
            isOneLineCommand = true;
            command.type = Syntax::Command::Include;
            return Action::Continue;
        }

        if (word == Syntax::Comment::OneLineProject)
        {
            isOneLineCommand = true;
            return Action::Continue;
        }

        // TODO: add code to check if first word was not recognized - then we can skip the
        // line?

        // Processing of comment meta data is done. Now we can proceed with parsing other
        // parts of text:

        // Handle commands in comments:
        if (isOneLineCommand or state->isProjectCommentBlock
            // TODO: c++20 modules
        )
        {
            command.append(word);
            return Action::Continue;
        }

        // Recognize interesting parts of C++ code:
        if (_cmd->isQuickMode() and
            (word == Syntax::CppKeywords::Class or word == Syntax::CppKeywords::Struct or
             word == Syntax::CppKeywords::Int or word == Syntax::CppKeywords::Char or
             word.starts_with(Syntax::CppKeywords::Main) or
             Tools::contains(word, Syntax::CppKeywords::DoubleColon) or
             Tools::contains(word, Syntax::CppKeywords::RoundBrackets)))
        {
            Log::debug("Finishing c++ file parsing early due to --quick flag");
            state->shouldFinish = true;
            isOneLineCommand = false;
            return Action::Break;
        }

        return Action::Continue;
    };

    for (const auto &character : std::as_const(line))
    {
        if (Tools::isWhitespace(character)) [[unlikely]]
        {
            if (word.empty())
            {
                // Skip indentation and long whitespace
                continue;
            }

            const auto action = processWord(word, state);

            if (action == Action::Break)
            {
                return;
            }
        }
        else [[likely]]
        {
            word.push_back(character);
        }
    }

    if (not word.empty())
    {
        processWord(word, state);
    }

    Log::verbose("Line is:", line);

    if (command.type == Syntax::Command::Unknown)
    {
        return;
    }

    if (command.isValid())
    {
        command.finalize();

        if (command.type == Syntax::Command::Include and
            not command.include().isLibrary and
            Tools::contains(_compiledFiles, command.value()))
        {
            return;
        }

        handleCommand(command, state);
    }
    else
    {
        // Skipping command handling when we are already parsing this file.
        // TODO: also skip parsing when this file was already parsed or compiled! Do not
        // duplicate the work!
    }
}

void Parser::handleCommand(Command command, CppState *state)
{
    Log::verbose("Handling command", command.whole());
    if (not command.isValid())
    {
        return;
    }

    Log::information("Found command:", command.whole());

    bool shouldParse = false;
    bool shouldAdd = false;

    if (command.type == Syntax::Command::Source)
    {
        shouldAdd = true;
        shouldParse = true;
    }
    else if (command.type == Syntax::Command::Library or
             command.type == Syntax::Command::Executable)
    {
        //if this is first Target command, and/ or it is issued in main.cpp, assume
        //it is naming the whole project and executable
        if (not _projectIdAlreadySet and state->id == _project.id)
        {
            Log::information("Autop-setting project name and executable name to:",
                             command.executable().name);
            const auto &commandId = _project.linkCommandIdFor(state->id);
            _project.commandRef(commandId).setExecutableName(command.executable().name);
            _projectIdAlreadySet = true;
        }
        else
        {
            shouldAdd = true;

            if (command.type == Syntax::Command::Library)
            {
                // TODO: wrong library name is parsed
                auto name = command.library().name;
                Log::verbose("Preparing library target:", name);

                // Command link;
                command.type = Syntax::Command::Library;
                command.parentId = _project.linkCommandIdFor(state->id);
                command.targetId = TargetId(std::move(name), TargetId::Type::Library);
                command.append(
                    std::filesystem::relative(command.targetId.name(), root()));
                command.finalize();

                // Link this library together with parent target
                auto linkCommand = &_project.commandRef(command.parentId);
                linkCommand->addLinkObject(command.object().name);

                // Ensure subsequent files are registered for compilation under this
                // library
                // TODO: go back to previous target when we go "out of scope"
                state->id = command.targetId;
            }
            else
            {
                // TODO
            }
        }
    }
    else if (command.type == Syntax::Command::Include)
    {
        shouldParse = true;

        // TODO: load library! If it is a gibs library

        // TODO: only parse if: not parsed already and it is a local library (part of the
        // same project)

        if (not command.include().path.empty())
        {
            addIncludePath(command.include().path);
        }
    }
    else if (command.type == Syntax::Command::Feature or
             command.type == Syntax::Command::Option)
    {
        // TODO: handle option
    }

    if (shouldAdd)
    {
        _project.addCommand(command);
    }

    const bool hasModifiers = command.hasModifiers();

    Log::verbose("Command:", command.whole(), "should parse:", shouldParse,
                 "has mods:", hasModifiers, "type:", Syntax::commandString(command.type),
                 "path:", command.path());

    if (shouldParse and hasModifiers)
    {
        const auto toFind = command.path();
        const auto pathOptional = findFile(toFind);

        if (pathOptional.has_value()) [[likely]]
        {
            Log::verbose("Found:", pathOptional.value());
        }
        else [[unlikely]]
        {
            Log::warning("Could not find file:", toFind);
            _compiledFiles.push_back(toFind);
            return;
        }

        const auto &path = pathOptional.value();

        if (std::filesystem::path(path).extension() == Syntax::Extension::ProjectFile)
        {
            parseProjectFile(path, state->id);
        }
        else if (command.type == Syntax::Command::Source)
        {
            parseCppFile(path, state->id);
        }
        else if (command.type == Syntax::Command::Include)
        {
            // Log::verbose("is library:", command.include.isLibrary, "is path to file:",
            // path);

            if (command.include().isLibrary)
            {
                // Note: this is temporary library name based on folder. A real name
                // needs to be used once it becomes known (when some library file is
                // parsed and contains the name)
                // TODO: make sure IDs don't get duplicated for this library, check if
                // this library and folder is already known
                TargetId libraryId(command.include().libraryName(),
                                   TargetId::Type::Library);

                if (Tools::isPathToFile(path))
                {
                    Log::verbose("Parsing", path, "as entry point of of library:",
                                 command.include().libraryName());
                    parseCppFile(path, libraryId);
                }
                else
                {
                    // Since we only have a path to a directory, we try to parse all
                    // files inside...
                    const std::filesystem::directory_entry dir(path);

                    for (auto const &it : std::filesystem::directory_iterator(dir))
                    {
                        if (it.is_regular_file())
                        {
                            Log::verbose("Parsing", it.path(),
                                         "to see if it is part of library:",
                                         command.include().libraryName());
                            parseCppFile(it.path(), libraryId);
                        }
                    }
                }
            }
            else if (Tools::isPathToFile(path.string()))
            {
                // Log::verbose("is header?", Tools::isHeaderFile(path), "file:", path);
                if (Tools::isHeaderFile(path))
                {
                    parseCppFile(path, state->id);
                }
                else
                {
                    const auto pathOptional = findCppFile(path);
                    if (pathOptional.has_value())
                    {
                        parseCppFile(pathOptional.value(), state->id);
                    }
                    else
                    {
                        Log::error("Included file not found:", path);
                    }
                }
            }
            else if (not path.empty())
            {
                addIncludePath(path);
            }
        }
    }

    // TODO: handle non-compilation commands

    // TODO: start running commands immediately
}

Syntax::FileType Parser::fileType(const std::filesystem::path &path) const
{
    const auto &extension = path.extension();

    if (extension == Syntax::Extension::ProjectFile)
    {
        return Syntax::FileType::Project;
    }
    else if (extension == Syntax::Extension::CppFile1 ||
             extension == Syntax::Extension::CppFile2)
    {
        return Syntax::FileType::Cpp;
    }
    else if (extension == Syntax::Extension::HeaderFile1 ||
             extension == Syntax::Extension::HeaderFile2 ||
             extension == Syntax::Extension::HeaderFile3)
    {
        return Syntax::FileType::H;
    }
    else if (extension == Syntax::Extension::ObjectFile1 ||
             extension == Syntax::Extension::ObjectFile2)
    {
        return Syntax::FileType::Object;
    }

    return Syntax::FileType::Other;
}

std::optional<std::filesystem::path> Parser::findFile(const std::string &name) const
{
    // TODO: add known files cache to speed things up

    Log::verbose("Looking for:", name);

    if (const std::filesystem::path current(root() / name);
        std::filesystem::exists(current))
    {
        return std::filesystem::relative(current, std::filesystem::current_path());
    }

    for (const auto &dir : _includePaths)
    {
        if (dir.empty()) [[unlikely]]
        {
            continue;
        }

        const auto current = root() / dir;

        for (auto const &it : std::filesystem::directory_iterator(current))
        {
            if (it.exists() && it.path().filename() == name)
            {
                return std::filesystem::relative(it, std::filesystem::current_path());
            }
        }
    }

    return {};
}

std::optional<std::filesystem::path> Parser::findCppFile(const std::string &name) const
{
    Log::verbose("Cpp looking for:", name);

    if (Tools::isHeaderFile(name))
    {
        // Replace extension with cpp extension and then proceed with search
        std::filesystem::path path = name;
        path.replace_extension(Syntax::Extension::CppFile1);

        if (auto option = findFile(path.string()); option.has_value())
        {
            Log::verbose("Found!", option.value());
            return option;
        }
        else
        {
            path.replace_extension(Syntax::Extension::CppFile2);

            if (auto option = findFile(path.string()); option.has_value())
            {
                Log::verbose("Found!", option.value());
                return option;
            }
        }

        // TODO: also look in other directories!
    }

    return {};
}

void Parser::addIncludePath(const std::filesystem::path &path)
{
    auto result = std::filesystem::relative(path, root());

    // TODO: more concrete detection of files: is it really a file or just a file-like dir
    // name?
    if (result.has_filename() && result.has_extension())
    {
        result = result.parent_path();
    }

    if (result.empty() or result == Dot)
    {
        return;
    }

    const auto findResult =
        std::find(_includePaths.cbegin(), _includePaths.cend(), result);

    if (findResult != _includePaths.cend())
    {
        return;
    }

    Log::information("Adding to include paths:", result);
    _includePaths.push_back(result);
}

const std::filesystem::path &Parser::root() const
{
    return _projectDirectory;
}
