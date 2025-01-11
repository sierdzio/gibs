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
}

AppError Parser::status() const
{
    return AppError();
}

void Parser::parse()
{
    // Take project name from parent directory - for now. It can be adjusted later if "target name"
    // command is found inside project files
    _project.id = TargetId(_projectDirectory.filename());

    if (_projectFile.has_filename()) {
        parseProjectFile(_projectFile, _project.id);
    }

    if (_projectEntryPoint.has_filename()) {
        parseCppFile(_projectEntryPoint, _project.id);
    }
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
        // Now, add compilation command for this cpp file:
        Command command;

        command.command = Syntax::Command::Source;
        command.append(path.string());

        Log::information("Compiling cpp file:", path.filename());
        _project.addCommand(command, id, Stage::First);
        // TODO: start running commands immediately

        // TODO: add this file to list of objects to be linked
        Log::information("Adding object file to linker command:", path.filename());
    } else if (type == Syntax::FileType::H) {
        Log::debug("Looking for a source file accompanying this header:", path.filename());

        for (auto const& it : std::filesystem::directory_iterator(path.parent_path())) {
            const auto &current = it.path();

            if (current.filename() == path.filename()) {
                const auto currentType = fileType(current);
                if (currentType == Syntax::FileType::Cpp) {
                    Command command;
                    command.command = Syntax::Command::Include;
                    command.append(current);

                    Log::information("Parsing cpp file for header:", path.filename());
                    _project.addCommand(command, id, Stage::First);
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

    const auto processWord = [&](auto& word, auto& state) -> Action
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
            command.append(word);
            return Action::Continue;
        }

        if (word == Syntax::Comment::OneLineProject)
        {
            isOneLineCommand = true;
            return Action::Continue;
        }

        // Processing of comment meta data is done. Now we can proceed with parsing other parts of text:

        // Handle commands in comments:
        if (isOneLineCommand
            || state->isProjectCommentBlock
            // TODO: c++20 modules
            )
        {
            command.append(word);
            return Action::Continue;
        }

        // Recognize interesting parts of C++ code:
        if (_cmd->isQuickMode()
            && (word == Syntax::CppKeywords::Class
                || word == Syntax::CppKeywords::Struct
                || word == Syntax::CppKeywords::Int
                || word == Syntax::CppKeywords::Char
                || word.starts_with(Syntax::CppKeywords::Main)))
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

    handleCommand(command, state->id);
}

void Parser::handleCommand(const Command& command, const TargetId& id)
{
    if (command.isValid() == false)
    {
        return;
    }

    Log::information("Found command:", command.whole());

    bool shouldParse = false;
    auto stage = Stage::Unknown;
    if (command.command == Syntax::Command::Source)
    {
        stage = Stage::First;
        shouldParse = true;
    }
    else if (command.command == Syntax::Command::Lib
            || command.command == Syntax::Command::Target)
    {
        stage = Stage::Second;
    }
    else if (command.command == Syntax::Command::Include)
    {
        shouldParse = true;
    }

    if (stage != Stage::Unknown)
    {
        _project.addCommand(command, id, stage);
    }

    if (shouldParse and not command.modifiers.empty())
    {
        const auto& path = command.modifiers.front();
        // TODO: add base path and such

        if (std::filesystem::path(path).extension() == Syntax::Extension::ProjectFile)
        {
            parseProjectFile(path, id);
        }
        else
        {
            parseCppFile(command.modifiers.front(), id);
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
