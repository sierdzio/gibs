#include "parser.h"
#include "cppstate.h"
#include "syntax.h"

#include "tools/commandline.h"
#include "tools/tools.h"
#include "tools/log.h"

#include "project/targetid.h"
#include "project/command.h"

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

        const auto extension = _input.extension();

        if (extension == Syntax::Extension::ProjectFile) {
            _projectFile = dir;
        } else if (extension == Syntax::Extension::CppFile1 || extension == Syntax::Extension::CppFile2) {
            _projectEntryPoint = dir;
        } else {
            Log::error("Input file type is incorrect: neither .gibs, nor a C++ source file:",
                      _input, "Extension is:", extension);
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
    // Nothing, for now.

    if (_projectFile.has_filename()) {
        parseProjectFile(_projectFile, {});
    }

    if (_projectEntryPoint.has_filename()) {
        parseCppFile(_projectEntryPoint, {});
    }
}

bool Parser::scanProjectDirectoryForEntryPoints()
{
    const auto &dir = _projectDirectory;

    for (auto const& it : std::filesystem::directory_iterator(dir)) {
        const auto extension = it.path().extension();

        if (extension == Syntax::Extension::ProjectFile) {
            _projectFile = it.path();
        } else if (extension == Syntax::Extension::CppFile1 || extension == Syntax::Extension::CppFile2) {
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
        Log::debug("Read:", line);
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
        Log::debug("Read:", line);
        parseCppLine(std::move(line), &state);

        if (state.shouldFinish) {
            break;
        }
    }

    file.close();

    const auto ext = path.extension();

    if (ext == Syntax::Extension::CppFile1 || ext == Syntax::Extension::CppFile2) {
        // Now, add compilation command for this cpp file:
        Command command;

        command.command = Syntax::Command::Source;
        command.value = path;

        Log::information("Compiling cpp file:", path.filename());
        _project.addCommand(command, id);
        // TODO: start running commands immediately

        // TODO: add this file to list of objects to be linked
        Log::information("Adding object file to linker command:", path.filename());
    } else if (ext == Syntax::Extension::HeaderFile1 || ext == Syntax::Extension::HeaderFile2
                || ext == Syntax::Extension::HeaderFile3) {
        // TODO: find source file and parse it
        Log::debug("Looking for a source file accompanying this header:", path.filename());

        for (auto const& it : std::filesystem::directory_iterator(path.parent_path())) {
            const auto &current = it.path();
            const auto &extension = current.extension();

            if (current.filename() == path.filename()) {
                if (extension == Syntax::Extension::CppFile1 || extension == Syntax::Extension::CppFile2) {
                    if (current.filename() == Syntax::Extension::Main) {
                        parseCppFile(current, state.id);
                    }
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

    if (command.isValid()) {
        Log::information("Found command:", command.whole());
        _project.addCommand(command, id);
        // TODO: start running commands immediately
    }
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
    bool isIncludeCommand = false;

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

            if (word == Syntax::Comment::MultilineEnd) {
                state->isCommentBlock = false;
                state->isProjectCommentBlock = false;
                continue;
            }

            // TODO: when a comment begins mid-line, finish existing Command

            // TODO: update TargetId properly!

            // Recognize project comments and comment blocks:
            if (word == Syntax::Comment::MultilineBeginProject) {
                state->isProjectCommentBlock = true;
                continue;
            } else if (word == Syntax::Comment::OneLine) {
                continue;
            } else if (word == Syntax::Comment::MultilineBegin) {
                // Recognize C++ comments and comment blocks:
                state->isCommentBlock = true;
                continue;
            }

            if (state->isCommentBlock) {
                // Skip C++ comments
                continue;
            }

            if (word == Syntax::Comment::OneLineProject) {
                isOneLineCommand = true;
            }

            // Processing of comment meta data is done. Now we can proceed with parsing other parts of text:

            // Handle commands in comments:
            if (isOneLineCommand || state->isProjectCommentBlock || isIncludeCommand) {
                command.append(word);
            }

            // Recognize interesting parts of C++ code:
            if (_cmd->isQuickMode() && (word == Syntax::CppKeywords::Class
                || word == Syntax::CppKeywords::Struct)) {
                state->shouldFinish = true;
                isOneLineCommand = false;
                return;
            }

            if (word == Syntax::CppKeywords::Include) {
                isIncludeCommand = true;
                command.append(Syntax::commandString(Syntax::Command::Include));
            }

            continue;
        }

        word.push_back(character);
    }

    if (command.isValid()) {
        Log::information("Found command:", command.whole());
        _project.addCommand(command, state->id);
        // TODO: start running commands immediately
    }
}
