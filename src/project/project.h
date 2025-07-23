#pragma once

#include "project/command.h"
#include "targetid.h"

#include <string>
#include <unordered_map>
#include <vector>

struct Project
{
    bool addCommand(const Command &command);

    CommandId linkCommandIdFor(const TargetId &id) const;
    Command &commandRef(const CommandId id);

    void logCommandTree() const;

    std::vector<Command> commands;
    TargetId id;

  private:
    void generateDepths();
    int depth(const Command &command) const;
    void logCommand(const int depth, const Command &command, std::string *string) const;

    std::unordered_map<CommandId, int> _commandDepths;
};
