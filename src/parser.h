#pragma once

#include "apperror.h"

#include <string>
#include <fstream>
#include <optional>
#include <filesystem>

class Parser {
public:
    Parser(std::string &&input);

    AppError status() const;

    void parse();

private:
    bool scanProjectDirectoryForEntryPoints();

    void parseProjectFile(const std::filesystem::path &path);
    void parseCppFile(const std::filesystem::path &path);
    void parseProjectLine(std::string &&line);

    std::filesystem::path _input;
    std::filesystem::path _projectDirectory;
    std::filesystem::path _projectFile;
    std::filesystem::path _projectEntryPoint;
    AppError _status = AppError::NoError;
};
