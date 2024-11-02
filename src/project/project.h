#pragma once

#include "target.h"

#include <vector>

struct Command;
struct TargetId;

struct Project
{
    bool addCommand(const Command &command, const TargetId &id);

    std::vector<Target> targets;
};
