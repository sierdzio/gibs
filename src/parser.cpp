#include "parser.h"
#include "syntax.h"
#include "log.h"

#include <iostream>

Parser::Parser(std::string &&input) : _input(std::move(input))
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

        if (extension == Extension::ProjectFile) {
            _projectFile = dir;
        } else if (extension != Extension::CppFile1 && extension != Extension::CppFile2) {
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
        parseProjectFile(_projectFile);
    }

    if (_projectEntryPoint.has_filename()) {
        parseCppFile(_projectEntryPoint);
    }
}

bool Parser::scanProjectDirectoryForEntryPoints()
{
    const auto &dir = _projectDirectory;

    for (auto const& it : std::filesystem::directory_iterator(dir)) {
        const auto extension = it.path().extension();

        if (extension == Extension::ProjectFile) {
            _projectFile = it.path();
        } else if (extension == Extension::CppFile1 || extension == Extension::CppFile2) {
            if (it.path().filename() == Extension::Main) {
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

void Parser::parseProjectFile(const std::filesystem::path &path)
{
    std::ifstream file(path, std::iostream::in);

    if (file.is_open() == false) {
        Log::error("Could not open file for reading:", path);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        Log::debug("Read:", line);
        parseProjectLine(std::move(line));
    }

    file.close();
}

void Parser::parseCppFile(const std::filesystem::path &path)
{
    std::ifstream file(path, std::iostream::in);

    if (file.is_open() == false) {
        Log::error("Could not open file for reading:", path);
        return;
    }

    std::string line;
    while (std::getline(file,line)) {
        Log::debug("Read:", line);
        // Recognize comments and comment blocks:

        // Recognize interesting parts of C++ code:
    }

    file.close();
}

void Parser::parseProjectLine(std::string &&line)
{
    if (line.size() > 0 && line.at(0) == Comment::Gibs) {
        Log::debug("Found a comment, ignoring...");
        return;
    }
}
