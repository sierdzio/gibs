#include "processor.h"
#include "compiler.h"
#include "linker.h"
#include "parsing/syntax.h"
#include "tool.h"
#include "tools/stringlist.h"
#include "tools/tools.h"

#include <logger/log.h>
#include <process/dryrunprocess.h>
#include <process/stupidprocess.h>

#include <future>
#include <memory>
#include <string>
#include <thread>

using namespace std::chrono_literals;

std::shared_future<void> Processor::schedule(const Command &command)
{
    Log::information("Scheduling command: ", command.whole());

    const auto typeString = Syntax::commandString(command.type);
    auto completion = std::make_shared<std::promise<void>>();
    auto future = completion->get_future().share();
    std::vector<std::unique_ptr<Process>> processes;

    switch (command.type)
    {
    case Syntax::Command::Library:
    case Syntax::Command::Executable:
        Log::debug("Processing:", typeString, "command:", command.whole());
        {
            const Linker tool(command, _compilerSet);

            for (const auto &toolCommand : tool.commands())
            {
                std::unique_ptr<Process> process;

                if (isDryRun())
                {
                    process = std::make_unique<DryRunProcess>();
                }
                else
                {
                    process = std::make_unique<StupidProcess>();
                }

                process->setExecutable(toolCommand.command);
                process->setArguments(toolCommand.arguments);
                process->setMetaInformation(std::to_string(command.id()) + " " +
                                            Syntax::commandString(command.type));

                process->setLogProcessOutput(isLogProcessOutput());
                process->start();
                processes.emplace_back(std::move(process));
            }
        }
        break;
    case Syntax::Command::Source:
        Log::debug("Processing:", typeString, "command:", command.whole());
        {
            const Compiler tool(command, _compilerSet);

            for (const auto &toolCommand : tool.commands())
            {
                std::unique_ptr<Process> process;

                if (isDryRun())
                {
                    process = std::make_unique<DryRunProcess>();
                }
                else
                {
                    process = std::make_unique<StupidProcess>();
                }

                process->setExecutable(toolCommand.command);
                process->setArguments(toolCommand.arguments);
                process->setMetaInformation(std::to_string(command.id()) + " " +
                                            Syntax::commandString(command.type));

                process->setLogProcessOutput(isLogProcessOutput());
                process->start();
                processes.emplace_back(std::move(process));
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
        completion->set_value();
        return future;
    case Syntax::Command::Option:
    case Syntax::Command::Include:
    case Syntax::Command::Feature:
    case Syntax::Command::Subproject:
    case Syntax::Command::Define:
    case Syntax::Command::Invalid:
    case Syntax::Command::Unknown:
        Log::warning("This command type:", typeString, "does not need to be processed");
        completion->set_value();
        return future;
    }

    if (processes.empty())
    {
        completion->set_value();
    }
    else
    {
        _runningCommands.push_back({command.id(), std::move(processes), completion});
    }

    return future;
}

void Processor::waitForFinished()
{
    if (_runningCommands.empty())
    {
        return;
    }

    forever
    {
        checkProcessStates();

        if (_runningCommands.empty())
        {
            Log::verbose("All processes have finished.");
            break;
        }
        else
        {
            std::this_thread::sleep_for(100ms);
        }
    }
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

void Processor::setCompilerSet(const CompilerSet &compilerSet)
{
    _compilerSet = compilerSet;
}

void Processor::checkProcessStates()
{
    for (size_t index = 0; index < _runningCommands.size(); /* nothing */)
    {
        auto &current = _runningCommands.at(index);

        for (size_t processIndex = 0; processIndex < current.processes.size();
             /* nothing */)
        {
            if (current.processes.at(processIndex)->isFinished())
            {
                current.processes.erase(current.processes.begin() +
                                        static_cast<long>(processIndex));
            }
            else
            {
                ++processIndex;
            }
        }

        if (current.processes.empty())
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
