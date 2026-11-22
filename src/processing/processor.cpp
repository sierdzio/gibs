#include "processor.h"
#include "compiler.h"
#include "exceptions/processexception.h"
#include "linker.h"
#include "parsing/syntax.h"
#include "tool.h"
#include "tools/stringlist.h"
#include "tools/tools.h"

#include <logger/log.h>
#include <process/dryrunprocess.h>
#include <process/process.h>

#include <algorithm>
#include <filesystem>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>

using namespace std::chrono_literals;

std::shared_future<void> Processor::schedule(const Command &command)
{
    Log::information("Scheduling command: ", command.whole());

    const auto typeString = Syntax::commandString(command.type);
    auto completion = std::make_shared<std::promise<void>>();
    auto future = completion->get_future().share();
    std::vector<std::unique_ptr<ProcessInterface>> processes;
    std::vector<CommandData> pendingCommands;
    const auto sequential =
        _compilerSet.commandExecution == CompilerSet::CommandExecution::Sequential;

    const auto completionForSkippedCommand = [&]() -> std::shared_future<void>
    {
        completion->set_value();
        return future;
    };

    const auto createProcess =
        [&](const CommandData &toolCommand) -> std::unique_ptr<ProcessInterface>
    {
        std::unique_ptr<ProcessInterface> process;

        if (isDryRun())
        {
            process = std::make_unique<DryRunProcess>();
        }
        else
        {
            process = std::make_unique<Process>();
        }

        process->setExecutable(toolCommand.command);
        process->setArguments(toolCommand.arguments);
        process->setMetaInformation(std::to_string(command.id()) + " " +
                                    std::string(Syntax::commandString(command.type)));
        process->setLogProcessOutput(isLogProcessOutput());

        return process;
    };

    switch (command.type)
    {
    case Syntax::Command::Library:
    case Syntax::Command::Executable:
    {
        const auto output = command.type == Syntax::Command::Executable
                                ? command.executable().outputPath
                                : std::filesystem::path(command.object().name);
        std::vector<std::string> dependencies;
        if (command.type == Syntax::Command::Executable)
        {
            dependencies.insert(dependencies.end(), command.executable().objects.begin(),
                                command.executable().objects.end());
            dependencies.insert(dependencies.end(),
                                command.executable().libraries.begin(),
                                command.executable().libraries.end());
        }
        else
        {
            dependencies.insert(dependencies.end(), command.library().objects.begin(),
                                command.library().objects.end());
        }

        const bool rebuiltDependency = std::any_of(
            dependencies.begin(), dependencies.end(),
            [&](const std::string &dependency)
            {
                std::error_code error;
                auto path =
                    std::filesystem::absolute(dependency, error).lexically_normal();
                return _rebuiltOutputs.contains(path.string());
            });
        if (not rebuiltDependency && _buildCache.isTargetUpToDate(output))
        {
            Log::verbose("Skipping target; output is up to date:", output.string());
            return completionForSkippedCommand();
        }
    }
        Log::debug("Processing:", typeString, "command:", command.whole());
        {
            const Linker tool(command, _compilerSet);
            const auto toolCommands = tool.commands();

            if (sequential && !toolCommands.empty())
            {
                processes.emplace_back(createProcess(toolCommands.front()));
                processes.back()->start();

                for (size_t index = 1; index < toolCommands.size(); ++index)
                {
                    pendingCommands.emplace_back(toolCommands.at(index));
                }
            }
            else
            {
                for (const auto &toolCommand : toolCommands)
                {
                    auto process = createProcess(toolCommand);
                    process->start();
                    processes.emplace_back(std::move(process));
                }
            }
        }
        break;
    case Syntax::Command::Source:
    {
        const auto &object = command.object();
        const auto source =
            object.sourcePath.empty()
                ? std::filesystem::absolute(std::filesystem::path(object.source))
                      .lexically_normal()
                : object.sourcePath.lexically_normal();
        const auto output = std::filesystem::path(object.name);
        if (_buildCache.isSourceUpToDate(source, output))
        {
            Log::verbose("Skipping source; object is up to date:", source.string());
            return completionForSkippedCommand();
        }
    }
        Log::debug("Processing:", typeString, "command:", command.whole());
        {
            const Compiler tool(command, _compilerSet);
            const auto toolCommands = tool.commands();

            if (_generateCompileCommands)
            {
                const auto &object = command.object();
                const auto file = object.sourcePath.empty()
                                      ? std::filesystem::path(object.source)
                                      : object.sourcePath;
                for (const auto &toolCommand : toolCommands)
                {
                    _compileCommands.add(std::filesystem::current_path(), file,
                                         toolCommand);
                }
            }

            if (sequential && !toolCommands.empty())
            {
                processes.emplace_back(createProcess(toolCommands.front()));
                processes.back()->start();

                for (size_t index = 1; index < toolCommands.size(); ++index)
                {
                    pendingCommands.emplace_back(toolCommands.at(index));
                }
            }
            else
            {
                for (const auto &toolCommand : toolCommands)
                {
                    auto process = createProcess(toolCommand);
                    process->start();
                    processes.emplace_back(std::move(process));
                }
            }
        }
        break;
    case Syntax::Command::Qt:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not implemented yet!");
        completion->set_value();
        return future;
    case Syntax::Command::Tool:
        Log::debug("Processing:", typeString, "command:", command.whole());
        {
            CommandData toolCommand;
            toolCommand.command = command.tool().executable;
            toolCommand.arguments = command.tool().arguments;

            auto process = createProcess(toolCommand);
            process->start();
            processes.emplace_back(std::move(process));
        }
        break;
    case Syntax::Command::Option:
    case Syntax::Command::Include:
    case Syntax::Command::Feature:
    case Syntax::Command::Configure:
    case Syntax::Command::Replace:
    case Syntax::Command::Subproject:
    case Syntax::Command::Tests:
    case Syntax::Command::Define:
    case Syntax::Command::Invalid:
    case Syntax::Command::Unknown:
        Log::warning("This command type:", typeString, "does not need to be processed");
        completion->set_value();
        return future;
    }

    const auto commandMeta = std::to_string(command.id()) + " " +
                             std::string(Syntax::commandString(command.type));

    if (processes.empty() && pendingCommands.empty())
    {
        completion->set_value();
    }
    else
    {
        _runningCommands.push_back(
            {command.id(), std::move(processes), std::move(pendingCommands), 0,
             _compilerSet.commandExecution, commandMeta, completion,
             command.type == Syntax::Command::Source
                 ? (command.object().sourcePath.empty()
                        ? std::filesystem::absolute(
                              std::filesystem::path(command.object().source))
                              .lexically_normal()
                        : command.object().sourcePath.lexically_normal())
                 : std::filesystem::path{},
             command.type == Syntax::Command::Source
                 ? std::filesystem::path(command.object().name)
             : command.type == Syntax::Command::Executable
                 ? command.executable().outputPath
                 : std::filesystem::path(command.object().name)});
    }

    return future;
}

void Processor::waitForFinished()
{
    while (not _runningCommands.empty())
    {
        checkProcessStates();

        if (_runningCommands.empty())
        {
            Log::verbose("All processes have finished.");
        }
        else
        {
            std::this_thread::sleep_for(100ms);
        }
    }

    if (_generateCompileCommands && not _compileCommands.write("compile_commands.json"))
    {
        Log::error("Failed to write compile_commands.json");
    }

    _buildCache.save();
}

void Processor::setDryRun(const bool dryRun)
{
    _dryRun = dryRun;
}

bool Processor::isDryRun() const
{
    return _dryRun;
}

void Processor::setLogProcessOutput(const bool enabled)
{
    _logProcessOutput = enabled;
}

bool Processor::isLogProcessOutput() const
{
    return _logProcessOutput;
}

void Processor::setGenerateCompileCommands(const bool enabled)
{
    _generateCompileCommands = enabled;
}

void Processor::setCompilerSet(const CompilerSet &compilerSet)
{
    _compilerSet = compilerSet;
}

void Processor::setBuildDirectory(const std::filesystem::path &buildDirectory)
{
    _buildCache.setDirectory(buildDirectory);
}

void Processor::checkProcessStates()
{
    for (size_t index = 0; index < _runningCommands.size(); /* nothing */)
    {
        auto &current = _runningCommands.at(index);

        for (size_t processIndex = 0; processIndex < current.processes.size();
             /* nothing */)
        {
            const auto &process = current.processes.at(processIndex);

            if (process->isFinished())
            {
                Tools::ScopeGuard logGuard(
                    [&]()
                    {
                        current.processes.erase(current.processes.begin() +
                                                static_cast<long>(processIndex));
                    });

                if (process->result().status == Exit::Status::Success)
                {
                    Log::information(
                        "Process finished successfully for command id:",
                        current.commandId, "executable:", process->executable(),
                        "arguments:", Tools::listToString(process->arguments()));

                    if (not isDryRun())
                    {
                        if (not current.sourcePath.empty())
                        {
                            _buildCache.markSourceCompiled(current.sourcePath,
                                                           current.outputPath);
                            if (not current.outputPath.empty())
                            {
                                _rebuiltOutputs.insert(
                                    std::filesystem::absolute(current.outputPath)
                                        .lexically_normal()
                                        .string());
                            }
                        }
                        else if (not current.outputPath.empty())
                        {
                            _buildCache.markTargetLinked(current.outputPath);
                            _rebuiltOutputs.insert(
                                std::filesystem::absolute(current.outputPath)
                                    .lexically_normal()
                                    .string());
                        }
                    }
                }
                else
                {
                    throw ProcessException(current.commandId, process.get());
                }
            }
            else
            {
                ++processIndex;
            }
        }

        if (current.processes.empty() &&
            current.commandExecution == CompilerSet::CommandExecution::Sequential &&
            current.nextCommandToStart < current.pendingCommands.size())
        {
            const auto &toolCommand =
                current.pendingCommands.at(current.nextCommandToStart++);

            std::unique_ptr<ProcessInterface> process;
            if (isDryRun())
            {
                process = std::make_unique<DryRunProcess>();
            }
            else
            {
                process = std::make_unique<Process>();
            }

            process->setExecutable(toolCommand.command);
            process->setArguments(toolCommand.arguments);
            process->setMetaInformation(current.commandMeta);
            process->setLogProcessOutput(isLogProcessOutput());
            process->start();
            current.processes.emplace_back(std::move(process));
        }

        if (current.processes.empty() &&
            (current.nextCommandToStart >= current.pendingCommands.size()))
        {
            current.completion->set_value();
            _runningCommands.erase(_runningCommands.begin() + static_cast<long>(index));
        }
        else
        {
            ++index;
        }
    }
}
