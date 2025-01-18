#pragma once

#include "syntax.h"
#include "project/project.h"
#include "tools/apperror.h"

#include <optional>
#include <string>
#include <filesystem>
#include <vector>

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

    std::optional<std::filesystem::path> findFile(const std::string &name) const;
    std::optional<std::filesystem::path> findCppFile(const std::string &name) const;

    std::filesystem::path _input;

    // TODO: move to Project?

    std::filesystem::path _projectDirectory;
    std::filesystem::path _projectFile;
    std::filesystem::path _projectEntryPoint;
    std::vector<std::filesystem::path> _includePaths;

    Project _project;

    // TODO: move to project?
    // TODO: separate list per-target and project; optimize lookup
    /*!
     * Of course, no header files are actually being compiled. But this keeps track of
     * header files for which a corresponding source file has already been found and
     * processed through handleCommand().
    */
    std::vector<std::string> _compiledHeaders;

    const CommandLine* _cmd = nullptr;

    AppError _status = AppError::NoError;
};
