#pragma once

#include "project/project.h"
#include "tools/apperror.h"

#include <string>
#include <filesystem>

class CommandLine;
struct CppState;
struct TargetId;
class Parser
{
public:
    Parser(const CommandLine *cmd);

    AppError status() const;

    void parse();

private:
    bool scanProjectDirectoryForEntryPoints();

    void parseProjectFile(const std::filesystem::path &path, const TargetId &id);
    void parseCppFile(const std::filesystem::path &path, const TargetId &id);

    void parseProjectLine(std::string &&line, const TargetId &id);
    void parseCppLine(std::string &&line, CppState *state);

    std::filesystem::path _input;
    std::filesystem::path _projectDirectory;
    std::filesystem::path _projectFile;
    std::filesystem::path _projectEntryPoint;

    Project _project;

    const CommandLine* _cmd = nullptr;

    AppError _status = AppError::NoError;
};
