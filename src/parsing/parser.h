#pragma once

#include "processing/command.h"
#include "tools/apperror.h"

#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <filesystem>

class CommandLine;
class Parser {
public:
    Parser(const CommandLine *cmd);

    AppError status() const;

    void parse();

private:
    struct CppState
    {
        bool isCommentBlock = false;
        bool isProjectCommentBlock = false;
        bool shouldFinish = false;
    };

    bool scanProjectDirectoryForEntryPoints();

    void parseProjectFile(const std::filesystem::path &path);
    void parseCppFile(const std::filesystem::path &path);

    void parseProjectLine(std::string &&line);
    void parseCppLine(std::string &&line, CppState *state);

    std::filesystem::path _input;
    std::filesystem::path _projectDirectory;
    std::filesystem::path _projectFile;
    std::filesystem::path _projectEntryPoint;

    std::vector<Command> _commands;

    const CommandLine* _cmd = nullptr;

    AppError _status = AppError::NoError;
};
