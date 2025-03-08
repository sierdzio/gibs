#include "project.h"
#include "command.h"
#include "tools/log.h"
#include <algorithm>
#include <vector>

bool Project::addCommand(const Command& command)
{
    commands.push_back(command);
    return true;
}

CommandId Project::linkCommandIdFor(const TargetId& id) const
{
    const auto it = std::find_if(commands.cbegin(), commands.cend(),
        [id](const Command& command) {
            return command.targetId == id;
        });

    return it == commands.cend() ? CommandId() : it->id();
}

/*!
 * Prints a tree showing all targets, commands and their relations.
 */
void Project::logCommandTree() const
{
    std::string result;

    result.append("All project commands:\n");

    for (const auto &command : commands)
    {
        if (command.parentId == 0)
        {
            result.append("|- " + command.whole() + '\n');

            // TODO: make this recursive to support deep trees
            for (const auto &sub : commands)
            {
                if (sub.id() == command.id())
                {
                    continue;
                }

                if (sub.parentId == command.id())
                {
                    result.append("  |- " + sub.whole() + '\n');
                }
            }
        }
    }

    Log::information(result);
}
