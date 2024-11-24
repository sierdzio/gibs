#pragma once

#include "syntax.h"
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

    void handleCommand(const Command& command, const TargetId& id);

    Syntax::FileType fileType(const std::filesystem::path& path) const;

    std::filesystem::path _input;
    std::filesystem::path _projectDirectory;
    std::filesystem::path _projectFile;
    std::filesystem::path _projectEntryPoint;

    Project _project;

    const CommandLine* _cmd = nullptr;

    AppError _status = AppError::NoError;
};
