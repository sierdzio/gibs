#pragma once

#include "target.h"

#include <vector>

struct Command;
struct TargetId;

struct Project
{
    void addDependency(const TargetId& target, const TargetId& dependency);
    bool addCommand(const Command &command, const TargetId& id, const Stage stage);

    std::vector<Target> targets;
    TargetId id;
};
