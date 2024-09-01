#include "parser.h"

#include <filesystem>

Parser::Parser(std::string &&input) : _input(std::move(input))
{
    if (_input.empty()) {
            _input = std::filesystem::current_path();
    }

    /*
     TODO:
     * check if _input exists
     * check if _input is a file or directory
     * if input is a directory, scan it to find gibs config files or main.cpp
     */

}