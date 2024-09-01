#pragma once

#include "apperror.h"

#include <string>
#include <filesystem>

class Parser {
public:
    Parser(std::string &&input);

    AppError status() const;

private:
    std::filesystem::path _input;
    std::filesystem::directory_entry _projectDirectory;
    std::filesystem::directory_entry _projectFile;
    std::filesystem::directory_entry _projectEntryPoint;
    AppError _status = AppError::NoError;
};