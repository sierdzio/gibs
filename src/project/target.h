#pragma once

#include "command.h"
#include "targetid.h"

#include <string>
#include <vector>

// TODO: pointers! Or?

struct CommandBundle
{
    std::vector<Command> commands;
};

struct Target
{
public:
    TargetId id;
    CommandBundle dependsOn;
    CommandBundle commands;
};
