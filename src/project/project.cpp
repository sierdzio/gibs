#include "project.h"
#include "command.h"
#include "exceptions/commanddepthexception.h"
#include "exceptions/commandnotfound.h"
#include "processing/processor.h"

#include <logger/log.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <future>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace
{
constexpr std::string Branch = "|--";
constexpr std::string Vertical = "|";
constexpr std::string Space = " ";
constexpr std::string Nl = "\n";
} //namespace

Project::Project(std::shared_ptr<Processor> processor) : _processor(processor)
{
}

bool Project::addCommand(const Command &command)
{
    commands.emplace_back(command);

    if (command.type == Syntax::Command::Source)
    {
        _commandCompletionFutures[command.id()] = _processor->schedule(commands.back());
        commands.back().setIsReadyToExecute(true);
    }

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

void Project::onParsingFinished()
{
    // Schedule libraries and executables based on the dependency graph that
    // was constructed while parsing. Each link command waits for its child
    // commands to complete before being scheduled.

    std::unordered_map<CommandId, std::vector<CommandId>> children;
    for (const auto &command : commands)
    {
        if (command.parentId != NullCommandId)
        {
            children[command.parentId].push_back(command.id());
        }
    }

    std::unordered_set<CommandId> pending;
    for (const auto &command : commands)
    {
        if (command.type == Syntax::Command::Library ||
            command.type == Syntax::Command::Executable)
        {
            pending.insert(command.id());
        }
    }

    while (not pending.empty())
    {
        bool madeProgress = false;

        // Update process states to ensure futures are set to ready
        _processor->checkProcessStates();

        for (const auto commandId :
             std::vector<CommandId>(pending.begin(), pending.end()))
        {
            const auto &command = commandRef(commandId);
            const auto childIt = children.find(commandId);
            bool dependenciesReady = true;

            if (childIt != children.cend())
            {
                for (const auto childId : childIt->second)
                {
                    const auto futureIt = _commandCompletionFutures.find(childId);
                    if (futureIt == _commandCompletionFutures.cend() ||
                        futureIt->second.wait_for(std::chrono::seconds(0)) !=
                            std::future_status::ready)
                    {
                        dependenciesReady = false;
                        break;
                    }
                }
            }

            if (not dependenciesReady)
            {
                continue;
            }

            _commandCompletionFutures[commandId] = _processor->schedule(command);
            commandRef(commandId).setIsReadyToExecute(true);
            pending.erase(commandId);
            madeProgress = true;
        }

        if (pending.empty())
        {
            break;
        }

        if (not madeProgress)
        {
            // If no progress was made, wait a bit before checking again
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    _processor->waitForFinished();
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

    if (commands.empty())
    {
        return;
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

    string->append(Branch + command.whole());

    if (not command.object().defines.empty())
    {
        string->append(" [defines:");
        for (const auto &define : command.object().defines)
        {
            string->append(" ");
            string->append(define);
        }
        string->append("]");
    }

    string->append(Nl);
}
