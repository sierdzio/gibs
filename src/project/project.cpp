#include "project.h"
#include "command.h"
#include "exceptions/commanddepthexception.h"
#include "exceptions/commandnotfound.h"

#include <logger/log.h>

#include <algorithm>
#include <cassert>

namespace
{
constexpr std::string Branch = "|--";
constexpr std::string Vertical = "|";
constexpr std::string Space = " ";
constexpr std::string Nl = "\n";
} //namespace

bool Project::addCommand(const Command &command)
{
    commands.emplace_back(command);
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

    result.append("All project commands:");
    result.append(Nl);

    for (const auto &command : commands)
    {
        logCommand(depth(command), command, &result);
    }

    Log::information(Log::Color(Log::Standard::Foreground::Green), result);
}

void Project::generateDepths()
{
    for (const auto &current : commands)
    {
        if (current.parentId == NullCommandId)
        {
            _commandDepths.insert({current.id(), 0});
            continue;
        }

        if (const auto parentIt = _commandDepths.find(current.parentId);
            parentIt == _commandDepths.cend())
        {
            throw CommandDepthException(current);
        }
        else
        {
            _commandDepths.insert({current.id(), parentIt->second + 1});
        }
    }
}

int Project::depth(const Command &command) const
{
    return _commandDepths.at(command.id());
}

void Project::logCommand(const int depth, const Command &command,
                         std::string *string) const
{
    if (depth > 0)
    {
        string->append(Vertical);
    }

    for (int i = 0; i < depth; ++i)
    {
        string->append(Space + Space);
    }

    string->append(Branch + command.whole() + Nl);
}
