#pragma once

#include "targetid.h"

#include <vector>

struct Command;

struct Project
{
    bool addCommand(const Command &command);

    std::vector<Command> commands;
    TargetId id;
};
