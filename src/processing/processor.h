#pragma once

#include "compilecommands.h"
#include "compilerset.h"
#include "project/command.h"
#include "tool.h"
#include "buildcache.h"

#include <future>
#include <memory>
//i include libraries/process
#include <process/processinterface.h>
#include <unordered_set>
#include <vector>

struct RunningCommand
{
    CommandId commandId;
    std::vector<std::unique_ptr<ProcessInterface>> processes;
    std::vector<CommandData> pendingCommands;
    std::size_t nextCommandToStart = 0;
    CompilerSet::CommandExecution commandExecution =
        CompilerSet::CommandExecution::Sequential;
    std::string commandMeta;
    std::shared_ptr<std::promise<void>> completion;
    std::filesystem::path sourcePath;
    std::filesystem::path outputPath;
};

class Processor
{
  public:
    Processor() = default;

    std::shared_future<void> schedule(const Command &command);
    void waitForFinished();

    void setDryRun(const bool dryRun);
    bool isDryRun() const;

    void setLogProcessOutput(const bool enabled);
    bool isLogProcessOutput() const;
    void setGenerateCompileCommands(const bool enabled);
    void setCompilerSet(const CompilerSet &compilerSet);
    void setBuildDirectory(const std::filesystem::path &buildDirectory);

    void checkProcessStates(); // Made public for Project to update futures

    std::vector<RunningCommand> _runningCommands;
    std::unordered_set<std::string> _rebuiltOutputs;
    BuildCache _buildCache;
    bool _dryRun = false;
    bool _logProcessOutput = false;
    bool _generateCompileCommands = true;
    CompilerSet _compilerSet = CompilerSet::defaultForPlatform();
    CompileCommands _compileCommands;
};
