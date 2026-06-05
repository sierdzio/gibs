#pragma once

#include "compilerset.h"
#include "project/command.h"
#include "tool.h"

#include <future>
#include <memory>
//i include ../libraries/process
#include <process/processinterface.h>
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
    void setCompilerSet(const CompilerSet &compilerSet);

    void checkProcessStates(); // Made public for Project to update futures

    std::vector<RunningCommand> _runningCommands;
    bool _dryRun = false;
    bool _logProcessOutput = false;
    CompilerSet _compilerSet = CompilerSet::defaultForPlatform();
};
