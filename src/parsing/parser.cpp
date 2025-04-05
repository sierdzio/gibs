#include "parser.h"
#include "cppstate.h"

#include "parsing/syntax.h"
#include "tools/commandline.h"
#include "tools/tools.h"
#include "tools/log.h"

#include "project/targetid.h"
#include "project/command.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cassert>

Parser::Parser(const CommandLine *cmd) : _input(cmd->input()), _cmd(cmd)
{
    if (_input.empty()) {
        _input = std::filesystem::current_path();
    }

    if (std::filesystem::exists(_input) == false) {
        Log::error("Input path does not exist, cannot continue:", _input);
        _status = AppError::WrongInputPath;
        return;
    }

    const auto dir = std::filesystem::directory_entry(_input);

    if (dir.is_regular_file()) {
        Log::information("Got a regular file ", _input);
        _projectEntryPoint = dir;
        const auto base = std::filesystem::path(_input).remove_filename();
        _projectDirectory = std::filesystem::directory_entry(base);

        const auto type = fileType(_input);

        if (type == Syntax::FileType::Project) {
            _projectFile = dir;
        } else if (type == Syntax::FileType::Cpp) {
            _projectEntryPoint = dir;
        } else {
            Log::error("Input file type is incorrect: neither .gibs, nor a C++ source file:",
                      _input, "Extension is:", _input.extension());
            _status = AppError::IncorrectInputFileType;
            return;
        }
    } else if (dir.is_directory()) {
        Log::information("Got a directory, will scan it for project files or main.cpp:", _input);
        _projectDirectory = dir;

        scanProjectDirectoryForEntryPoints();
    }

    _includePaths.push_back(_projectDirectory);
}

AppError Parser::status() const
{
    return AppError();
}

void Parser::parse()
{
    // Take project name from parent directory - for now. It can be adjusted later if "target name"
    // command is found inside project files
    _project.id = TargetId(_projectDirectory.parent_path().filename(),
                           TargetId::Type::Executable);

    Log::information("Project name:", _project.id.name());

    if (_projectFile.has_filename()) {
        parseProjectFile(_projectFile, _project.id);
    }

    if (_projectEntryPoint.has_filename()) {
        Command link;
        link.targetId = _project.id;
        // TODO: executable or library or just target - decide
        link.type = Syntax::Command::Executable;
        link.modifiers.push_back(_project.id.name());
        link.finalize();
        _project.addCommand(link);
        parseCppFile(_projectEntryPoint, _project.id);
    }

    _project.logCommandTree();
}

bool Parser::scanProjectDirectoryForEntryPoints()
{
    const auto &dir = _projectDirectory;

    for (auto const& it : std::filesystem::directory_iterator(dir)) {
        const auto type = fileType(it.path());

        if (type == Syntax::FileType::Project) {
            _projectFile = it.path();
        } else if (type == Syntax::FileType::Cpp) {
            if (it.path().filename() == Syntax::Extension::Main) {
                _projectEntryPoint = it.path();
            }
        }
    }

    if (std::filesystem::directory_entry(_projectFile).exists()
        || std::filesystem::directory_entry(_projectEntryPoint).exists()) {
        return true;
    }

    Log::error("Neither .gibs, nor a C++ source file has been found in directory:", _projectDirectory);

    _status = AppError::EntryPointNotFound;
    return false;
}

void Parser::parseProjectFile(const std::filesystem::path &path, const TargetId &id)
{
    std::ifstream file(path, std::iostream::in);

    if (file.is_open() == false) {
        Log::error("Could not open file for reading:", path);
        return;
    }

    Log::debug("Reading project file:", path);

    std::string line;
    // TODO: implement a custom file reading routine to read it character by character and parse on the fly
    while (std::getline(file, line)) {
        Log::verbose("Read:", line);
        parseProjectLine(std::move(line), id);
    }

    file.close();
}

void Parser::parseCppFile(const std::filesystem::path &path, const TargetId &id)
{
    std::ifstream file(path, std::iostream::in);

    if (file.is_open() == false) {
        Log::error("Could not open file for reading:", path);
        return;
    }

    Log::debug("Reading C++ file:", path);

    CppState state;
    state.id = id;

    std::string line;

    // TODO: implement a custom file reading routine to read it character by character and parse on the fly
    while (std::getline(file,line)) {
        Log::verbose("Read:", line);
        parseCppLine(std::move(line), &state);

        if (state.shouldFinish) {
            break;
        }
    }

    file.close();

    const auto type = fileType(path);

    if (type == Syntax::FileType::Cpp) {
        // Prepare link command if not already present:
        auto linkId = _project.linkCommandIdFor(id);
        if (linkId == 0) {
            Command link;
            link.type = id.type() == TargetId::Type::Executable
                                     ? Syntax::Command::Executable
                                     : Syntax::Command::Library;
            link.targetId = id;
            link.append(id.name());
            link.finalize();
            _project.addCommand(link);
        }

        auto linkCommand = &_project.commandRef(linkId);

        // Now, add compilation command for this cpp file:
        Command compile;
        compile.type = Syntax::Command::Source;
        compile.append(path.string());
        compile.targetId = id;
        compile.parentId = linkCommand->id();
        compile.finalize();

        Log::debug("Adding object file to linker command:", compile.object.name);

        if (linkCommand->type == Syntax::Command::Executable)
        {
            linkCommand->executable.objects.push_back(compile.object.name);
        }
        else if (linkCommand->type == Syntax::Command::Library)
        {
            linkCommand->library.objects.push_back(compile.object.name);
        }

        Log::information("Compiling cpp file:", path.filename());
        _project.addCommand(compile);
        _processor.schedule(compile);

        // TODO: only execute this command after all children have finished processing!
        //_processor.schedule(link);
    } else if (type == Syntax::FileType::H) {
        Log::debug("Looking for a source file accompanying this header:", path.filename());

        for (auto const& it : std::filesystem::directory_iterator(path.parent_path())) {
            const auto &current = it.path();

            if (current.filename() == path.filename()) {
                const auto currentType = fileType(current);
                if (currentType == Syntax::FileType::Cpp) {
                    Command command;
                    command.type = Syntax::Command::Include;
                    command.append(current);
                    command.targetId = id;
                    command.parentId = _project.linkCommandIdFor(id);

                    Log::information("Parsing cpp file for header:", path.filename());
                    handleCommand(command, id);
                    break;
                }
            }
            // TODO: handle case where source file is in a different directory... maybe cache the dir structure ?
        }
    }
}

void Parser::parseProjectLine(std::string &&line, const TargetId &id)
{
    if (line.size() == 0) {
        return;
    }

    if (line.size() > 0 && line.at(0) == Syntax::Comment::Project) {
        Log::debug("Found a comment, ignoring...");
        return;
    }

    std::string word;
    Command command;

    for (const auto &character : std::as_const(line)) {
        if (character == ' ' || character == '\t') {
            if (word.empty() == true) {
                // Skip indentation and long whitespace
                continue;
            }

            // Make sure word gets cleaned up even if we exit early
            const auto guard = Tools::ScopeGuard([&word]{
                word.clear();
            });

            if (word.size() == 1 && word.at(0) == Syntax::Comment::Project) {
                // Skip comment line
                continue;
            }

            command.append(word);
        }
    }

    handleCommand(command, id);
}

void Parser::parseCppLine(std::string &&line, CppState *state)
{
    assert(state);

    if (line.size() == 0) {
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

    const auto processWord = [&](std::string& word, auto& state) -> Action
    {
        // Make sure word gets cleaned up even if we exit early
        const auto guard = Tools::ScopeGuard([&word]{
            word.clear();
        });

        if (word == Syntax::Comment::MultilineEnd)
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
        else if (word == Syntax::Comment::MultilineBegin)
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

        // TODO: add code to check if first word was not recognized - then we can skip the line?

        // Processing of comment meta data is done. Now we can proceed with parsing other parts of text:

        if (command.type == Syntax::Command::Include)
        {
            if (word.starts_with(Syntax::CppKeywords::OpenLibraryInclude))
            {
                // TODO: parse library header
                command.type = Syntax::Command::Invalid;
                return Action::Break;
            }
        }

        // Handle commands in comments:
        if (isOneLineCommand or state->isProjectCommentBlock
            // TODO: c++20 modules
            )
        {
            command.append(word);
            return Action::Continue;
        }

        // Recognize interesting parts of C++ code:
        if (_cmd->isQuickMode()
            and (word == Syntax::CppKeywords::Class
                or word == Syntax::CppKeywords::Struct
                or word == Syntax::CppKeywords::Int
                or word == Syntax::CppKeywords::Char
                or word.starts_with(Syntax::CppKeywords::Main)
                or Tools::contains(word, Syntax::CppKeywords::DoubleColon)
                or Tools::contains(word, Syntax::CppKeywords::RoundBrackets)
                ))
        {
            Log::debug("Finishing c++ file parsing early due to --quick flag");
            state->shouldFinish = true;
            isOneLineCommand = false;
            return Action::Break;
        }

        return Action::Continue;
    };

    for (const auto &character : std::as_const(line)) {
        if (character == ' ' || character == '\t') {
            if (word.empty()) {
                // Skip indentation and long whitespace
                continue;
            }

            const auto action = processWord(word, state);

            if (action == Action::Break)
            {
                break;
            }
        }
        else
        {
            word.push_back(character);
        }
    }

    if (not word.empty())
    {
        processWord(word, state);
    }

    if (command.isValid())
    {
        command.finalize();

        if (command.type == Syntax::Command::Include
            and not command.include.isLibrary
            and Tools::contains(_compiledHeaders, command.value()))
        {
            return;
        }

        handleCommand(command, state->id);
    }
    else
    {
        // Skipping command handling when we are already parsing this file.
        // TODO: also skip parsing when this file was already parsed or compiled! Do not duplicate the work!
    }
}

void Parser::handleCommand(const Command& command, const TargetId& id)
{
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
    else if (command.type == Syntax::Command::Library
            or command.type == Syntax::Command::Executable)
    {
        // If this is first Target command, and/ or it is issued in main.cpp, assume
        // it is naming the whole project and executable
        if (id == _project.id)
        {
            const auto& commandId = _project.linkCommandIdFor(id);
            _project.commandRef(commandId).executable.name = command.executable.name;
        }
        else
        {
            shouldAdd = true;
        }
    }
    else if (command.type == Syntax::Command::Include)
    {
        if (command.include.isLibrary)
        {
            // TODO: load library! If it is a gibs library

            // TODO: only parse if: not parsed already and it is a local library (part of the same project)
            shouldParse = true;

            if (command.include.isPathToFile())
            {
                _compiledHeaders.push_back(command.include.path);
            }
            else
            {
                _includePaths.push_back(command.include.libraryDirPath());
            }
        }
        else
        {
            if (command.include.isPathToFile())
            {
                shouldParse = true;
                _compiledHeaders.push_back(command.value());
            }
            else
            {
                _includePaths.push_back(command.include.libraryDirPath());
            }
        }
    }
    else if (command.type == Syntax::Command::Feature
            or command.type == Syntax::Command::Option)
    {
        // TODO: handle option
    }

    if (shouldAdd)
    {
        _project.addCommand(command);
    }

    if (shouldParse and not command.modifiers.empty())
    {
        const auto& path = command.modifiers.front();
        // TODO: add base path and such

        if (std::filesystem::path(path).extension() == Syntax::Extension::ProjectFile)
        {
            parseProjectFile(path, id);
        }
        else if (command.type == Syntax::Command::Include && command.include.isLibrary)
        {
            // Note: this is temporary library name based on folder. A real name needs to
            // be used once it becomes known (when some library file is parsed and contains
            // the name)
            // TODO: make sure IDs don't get duplicated for this library, check if this library
            // and folder is already known
            TargetId libraryId(command.include.libraryName(), TargetId::Type::Library);

            if (command.include.isPathToFile() && std::filesystem::exists(command.include.path))
            {
                Log::verbose("Parsing", command.include.path,
                             "as entry point of of library:", command.include.libraryName());
                parseCppFile(root() / command.include.path, libraryId);
            }
            else
            {
                // Since we only have a path to a directory, we try to parse all files inside...
                const std::filesystem::directory_entry dir(root() / command.include.path);

                for (auto const& it : std::filesystem::directory_iterator(dir))
                {
                    if (it.is_regular_file())
                    {
                        Log::verbose("Parsing", it.path(),
                                     "to see if it is part of library:",
                                     command.include.libraryName());
                        _compiledHeaders.push_back(it.path());
                        parseCppFile(it.path(), libraryId);
                    }
                }
            }
        }
        else
        {
            const auto pathOptional = findCppFile(command.modifiers.front());
            if (pathOptional.has_value())
            {
                parseCppFile(pathOptional.value(), id);
            }
            else
            {
                Log::error("Included file not found:", command.modifiers.front());
            }
        }
    }

    // TODO: handle non-compilation commands

    // TODO: start running commands immediately
}

Syntax::FileType Parser::fileType(const std::filesystem::path &path) const
{
    const auto& extension = path.extension();

    if (extension == Syntax::Extension::ProjectFile)
    {
        return Syntax::FileType::Project;
    }
    else if (extension == Syntax::Extension::CppFile1
            || extension == Syntax::Extension::CppFile2)
    {
        return Syntax::FileType::Cpp;
    }
    else if (extension == Syntax::Extension::HeaderFile1
            || extension == Syntax::Extension::HeaderFile2
            || extension == Syntax::Extension::HeaderFile3)
    {
        return Syntax::FileType::H;
    }
    else if (extension == Syntax::Extension::ObjectFile1
            || extension == Syntax::Extension::ObjectFile2)
    {
        return Syntax::FileType::Object;
    }

    return Syntax::FileType::Other;
}

std::optional<std::filesystem::path> Parser::findFile(const std::string &name) const
{
    // TODO: add known files cache to speed things up

    Log::verbose("Looking for:", name);

    for  (const auto& dir : _includePaths)
    {
        for (auto const& it : std::filesystem::directory_iterator(root() / dir))
        {
            if (it.exists() && it.is_regular_file() && it.path().filename() == name) {
                return it;
            }
        }
    }

    return {};
}

std::optional<std::filesystem::path> Parser::findCppFile(const std::string &name) const
{
    Log::verbose("Looking for CPP file for:", name);

    if (name.ends_with(Syntax::Extension::HeaderFile1)
        || name.ends_with(Syntax::Extension::HeaderFile2)
        || name.ends_with(Syntax::Extension::HeaderFile3))
    {
        // TODO: replace extension with cpp extension and then proceed with search
        std::filesystem::path path = name;
        path.replace_extension(Syntax::Extension::CppFile1);

        if (auto option = findFile(path.string()); option.has_value())
        {
            return option;
        }
        else
        {
            path.replace_extension(Syntax::Extension::CppFile2);
            auto checker = std::filesystem::directory_entry(path);

            if (auto option = findFile(path.string()); option.has_value())
            {
                return option;
            }
        }
    }

    return {};
}

const std::filesystem::path& Parser::root() const
{
    return _projectDirectory;
}
