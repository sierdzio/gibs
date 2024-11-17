#pragma once

#include "command.h"
#include "targetid.h"

#include <string>
#include <vector>

// TODO: pointers! Or?

enum class Stage
{
    Unknown,
    First,
    Second
};

struct CommandBundle
{
    std::vector<Command> commands;
};

struct Target
{
public:
    TargetId id;
    std::vector<TargetId> dependencies;
    CommandBundle stageOne;
    CommandBundle stageTwo;
};
