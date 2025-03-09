#pragma once

#include "project/command.h"
#include "targetid.h"

#include <vector>

struct Command;

struct Project
{
    bool addCommand(const Command &command);

    CommandId linkCommandIdFor(const TargetId& id) const;
    Command& commandRef(const CommandId id);

    void logCommandTree() const;

    std::vector<Command> commands;
    TargetId id;
};
