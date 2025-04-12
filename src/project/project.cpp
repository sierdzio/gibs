#include "project.h"
#include "command.h"
#include "exceptions/commandnotfound.h"
#include "tools/log.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace
{
constexpr auto EntryMark = "|- ";
constexpr auto Vertical = '|';
} //namespace

bool Project::addCommand(const Command &command)
{
    commands.push_back(command);
    return true;
}

CommandId Project::linkCommandIdFor(const TargetId &id) const
{
    const auto it =
        std::find_if(commands.cbegin(), commands.cend(),
                     [id](const Command &command) { return command.targetId == id; });

    return it == commands.cend() ? CommandId() : it->id();
}

Command &Project::commandRef(const CommandId id)
{
    for (auto &current : commands)
    {
        if (current.id() == id)
        {
            return current;
        }
    }

    throw CommandNotFound(id, commands);
}

/*!
 * Prints a tree showing all targets, commands and their relations.
 */
void Project::logCommandTree() const
{
    if (_commandDepths.empty())
    {
        // Naughty!
        const_cast<Project *>(this)->generateDepths();
    }

    std::string result;

    result.append("All project commands:\n");

    for (const auto &command : commands)
    {
        result = logSubTree(depth(command), command, std::move(result));
    }

    Log::information(result);
}

void Project::generateDepths()
{
    for (const auto &current : commands)
    {
        if (current.parentId == 0)
        {
            _commandDepths.insert({current.id(), 0});
            continue;
        }

        const auto it = _commandDepths.find(current.parentId);

        if (it != _commandDepths.cend())
        {
            _commandDepths.insert({current.id(), it->second + 1});
        }
    }
}

int Project::depth(const Command &command) const
{
    return _commandDepths.at(command.id());
}

std::string Project::logSubTree(const int depth, const Command &command,
                                std::string &&string) const
{
    if (depth > 0)
    {
        string.push_back(Vertical);
    }

    for (int i = 0; i < depth; ++i)
    {
        string.push_back(' ');
    }

    string.append(EntryMark + command.whole() + '\n');

    // TODO: make this recursive to support deep trees
    for (const auto &sub : commands)
    {
        if (sub.id() == command.id())
        {
            continue;
        }

        if (sub.parentId == command.id())
        {
            string.append("  |- " + sub.whole() + '\n');
        }
    }

    return string;
}
