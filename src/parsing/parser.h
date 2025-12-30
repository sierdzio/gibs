#pragma once

#include "syntax.h"
#include "project/command.h"
#include "tools/apperror.h"
#include "tools/stringlist.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <memory>

class CommandLine;
class Project;
struct CppState;
struct TargetId;

class Parser
{
  public:
    Parser(const std::filesystem::path& inputPath, const bool isQuickMode, std::shared_ptr<Project> project);

    AppError status() const;

    void parse();

  private:
    bool scanProjectDirectoryForEntryPoints();

    void parseProjectFile(const std::filesystem::path &path, const TargetId &id);
    void parseCppFile(const std::filesystem::path &path, const TargetId &id);

    void parseProjectLine(std::string &&line, const TargetId &id);
    void parseCppLine(std::string &&line, CppState *state);

    // TODO: move command instead of copying
    void handleCommand(Command command, CppState *state);

    Syntax::FileType fileType(const std::filesystem::path &path) const;

    std::optional<std::filesystem::path> findFile(const std::string &name) const;
    std::optional<std::filesystem::path> findCppFile(const std::string &name) const;
    void addIncludePath(const std::filesystem::path &path);
    const std::filesystem::path &root() const;

    // TODO: move to Project?

    std::filesystem::path _projectDirectory;
    std::filesystem::path _projectFile;
    std::filesystem::path _projectEntryPoint;
    // TODO: should be per target (library, executable) or even more granular to speed things up?
    std::vector<std::filesystem::path> _includePaths;

    // TODO: move to project?
    // TODO: separate list per-target and project; optimize lookup
    /*!
     * Of course, no header files are actually being compiled. But this keeps track of
     * header files for which a corresponding source file has already been found and
     * processed through handleCommand().
     */
    StringList _compiledFiles;

    std::shared_ptr<Project> _project;

    AppError _status = AppError::NoError;
    bool _projectIdAlreadySet = false;
    bool _isQuickMode = false;
};
