#pragma once

#include "apperror.h"

#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <filesystem>

// TODO: move to a separate .h, .cpp pair
struct Command
{
    bool isValid() const;
    std::vector<std::string> whole;
};

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
