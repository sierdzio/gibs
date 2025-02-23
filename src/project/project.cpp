#include "project.h"

#include "command.h"

bool Project::addCommand(const Command& command)
{
    commands.push_back(command);
    return true;
}
