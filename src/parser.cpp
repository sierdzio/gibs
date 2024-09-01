#include "parser.h"

#include <iostream>

Parser::Parser(std::string &&input) : _input(std::move(input))
{
    if (_input.empty()) {
        _input = std::filesystem::current_path();
    }

    /*
     TODO:
     * check if _input is a file or directory
     * if input is a directory, scan it to find gibs config files or main.cpp
     */

    if (std::filesystem::exists(_input) == false) {
        std::cout << "Input path does not exist, cannot continue: " << _input << std::endl;
        _status = AppError::WrongInputPath;
        return;
    }

    const auto dir = std::filesystem::directory_entry(_input);

    if (dir.is_regular_file()) {
        std::cout << "Got a regular file " << _input << std::endl;
        _projectEntryPoint = dir;
        const auto base = std::filesystem::path(_input).remove_filename();
        _projectDirectory = std::filesystem::directory_entry(base);

        const auto extension = _input.extension();

        if (extension == ".gibs") {
            _projectFile = dir;
        } else if (extension != ".cpp" && extension != ".cxx") {
            std::cout << "Input file type is incorrect: neither .gibs, nor a C++ source file: " 
            << _input << " Extension is: " << extension << std::endl;
            _status = AppError::IncorrectInputFileType;
            return;
        }
    } else if (dir.is_directory()) {
        std::cout << "Got a directory, will scan it for project files or main.cpp" << _input << std::endl;
        _projectDirectory = dir;
    }
}

AppError Parser::status() const
{
    return AppError();
}
