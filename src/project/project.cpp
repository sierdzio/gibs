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

bool isSubdirectoryOf(const std::filesystem::path &path,
                      const std::filesystem::path &base)
{
    const auto normalizedPath = path.lexically_normal();
    const auto normalizedBase = base.lexically_normal();
    const auto relative = normalizedPath.lexically_relative(normalizedBase);

    if (relative.empty())
    {
        return false;
    }

    const auto firstElement = *relative.begin();
    return firstElement != ".." && firstElement != ".";
}
} //namespace

Project::Project(std::shared_ptr<Processor> processor) : _processor(processor)
{
}

bool Project::addCommand(const Command &command)
{
    commands.emplace_back(command);

    const auto &added = commands.back();
    std::filesystem::path output;
    if (added.type == Syntax::Command::Source)
    {
        output = added.object().name;
    }
    else if (added.type == Syntax::Command::Executable)
    {
        output = added.executable().outputPath;
    }
    else if (added.type == Syntax::Command::Library)
    {
        output = added.object().name;
    }

    if (not output.empty())
    {
        std::error_code outputDirectoryError;
        std::filesystem::create_directories(output.parent_path(), outputDirectoryError);
        if (outputDirectoryError)
        {
            Log::error("Could not create output directory:", output.parent_path());
        }
    }

    if (command.type == Syntax::Command::Source)
    {
        _commandCompletionFutures[command.id()] = _processor->schedule(commands.back());
        commands.back().setIsReadyToExecute(true);
    }

    return true;
}

CommandId Project::linkCommandIdFor(const TargetId &id,
                                    const std::filesystem::path &path) const
{
    const auto directory =
        std::filesystem::is_directory(path) ? path : path.parent_path();

    const auto comparator = [id, path](const Command &command)
    {
        const auto idMatches = command.targetId == id;

        if (command.type == Syntax::Command::Library or
            command.type == Syntax::Command::Executable)
        {
            return idMatches;
        }

        if (not idMatches)
        {
            return false;
        }

        const auto isSubdir = isSubdirectoryOf(path, command.targetId.rootDirectory());
        return isSubdir;
    };

    const auto it = std::find_if(commands.cbegin(), commands.cend(), comparator);

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
        // TODO: perhaps catch the exception here and stop the process if any process
        // failed
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

    // Group link commands (libraries/executables) and their source files by target
    auto keyOf = [](const TargetId &t)
    { return t.name() + ":" + TargetId::typeString(t.type()); };

    std::unordered_map<std::string, const Command *> linkByKey;
    std::vector<const Command *> linkCommands;
    for (const auto &command : commands)
    {
        if (command.type == Syntax::Command::Library ||
            command.type == Syntax::Command::Executable)
        {
            const auto key = keyOf(command.targetId);
            linkByKey[key] = &command;
            linkCommands.push_back(&command);
        }
    }

    std::unordered_map<std::string, std::vector<const Command *>> sourcesByKey;
    std::vector<const Command *> unassignedSources;
    for (const auto &command : commands)
    {
        if (command.type == Syntax::Command::Source)
        {
            const auto key = keyOf(command.targetId);
            if (linkByKey.find(key) != linkByKey.end())
            {
                sourcesByKey[key].push_back(&command);
            }
            else
            {
                unassignedSources.push_back(&command);
            }
        }
    }

    // Build parent->children mapping among link commands only
    std::unordered_map<CommandId, std::vector<const Command *>> linkChildren;
    std::vector<const Command *> linkRoots;
    for (const auto *link : linkCommands)
    {
        if (link->parentId == NullCommandId)
        {
            linkRoots.push_back(link);
        }
        else
        {
            linkChildren[link->parentId].push_back(link);
        }
    }

    auto sortByWhole = [](std::vector<const Command *> &vec)
    {
        std::sort(vec.begin(), vec.end(), [](const Command *a, const Command *b)
                  { return a->whole() < b->whole(); });
    };

    // Print all link commands in a stable order: executables first, then libraries
    std::sort(linkCommands.begin(), linkCommands.end(),
              [](const Command *a, const Command *b)
              {
                  if (a->type != b->type)
                  {
                      return a->type == Syntax::Command::Executable;
                  }
                  return a->whole() < b->whole();
              });

    for (const auto *link : linkCommands)
    {
        logCommand(depth(*link), *link, &result);

        // Print source files that belong to this target
        const auto key = keyOf(link->targetId);
        if (const auto it = sourcesByKey.find(key); it != sourcesByKey.end())
        {
            auto files = it->second; // copy to sort
            sortByWhole(files);
            for (const auto *fileCmd : files)
            {
                logCommand(depth(*fileCmd), *fileCmd, &result);
            }
        }
    }

    // Print any unassigned source files (fallback)
    if (!unassignedSources.empty())
    {
        sortByWhole(unassignedSources);
        for (const auto *fileCmd : unassignedSources)
        {
            logCommand(depth(*fileCmd), *fileCmd, &result);
        }
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
