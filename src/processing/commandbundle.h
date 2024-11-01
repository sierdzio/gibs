#pragma once

#include "command.h"

#include <vector>

class CommandBundle
{
public:
    std::vector<Command> stageOneCommands;
    std::vector<Command> stageTwoCommands;
};
