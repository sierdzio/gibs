#pragma once

#include "processing/command.h"

class Processor
{
public:
    Processor();

    void schedule(const Command& command);
};
