#pragma once

#include "project/command.h"

#include <process/process.h>
#include <unordered_map>

class Processor
{
  public:
    Processor();

    void schedule(const Command &command);

  private:
    std::unordered_map<CommandId, Process *> processes;
};
