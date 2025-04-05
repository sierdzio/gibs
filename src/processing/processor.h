#pragma once

#include "project/command.h"

class Processor
{
  public:
    Processor();

    void schedule(const Command &command);
};
