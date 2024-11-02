#include "project.h"

#include "command.h"
#include "targetid.h"

bool Project::addCommand(const Command& command, const TargetId& id)
{
    for (auto &target : targets) {
        if (target.id == id)
        {
            if (command.command == Syntax::Command::Source)
            {
                target.dependsOn.commands.push_back(command);
            } else {
                target.commands.commands.push_back(command);
            }
        }
    }

    return false;
}
