#include "project.h"
#include "command.h"
#include "tools/log.h"
#include <vector>

bool Project::addCommand(const Command& command)
{
    commands.push_back(command);
    return true;
}

/*!
 * Prints a tree showing all targets, commands and their relations.
 */
void Project::logCommandTree() const
{
    std::string result;

    result.append("All project commands:\n");

    std::vector<Command> topLevel;



    Log::information(result);
}
