#pragma once

#include "project/command.h"

#include <memory>
#include <process/process.h>
#include <vector>

struct RunningProcess
{
    CommandId commandId;
    std::unique_ptr<Process> process;
};

class Processor
{
  public:
    Processor();

    void schedule(const Command &command);
    void waitForFinished();

    void setDryRun(const bool dryRun);
    bool isDryRun() const;

  private:
    void checkProcessStates();

    std::vector<RunningProcess> _processes;
    bool _dryRun = false;
};
