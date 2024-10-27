#pragma once

#include "parsing/command.h"

class Processor
{
public:
    Processor();

    void schedule(const Command& command);
};
