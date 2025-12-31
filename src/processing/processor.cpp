#include "processor.h"
#include "compiler.h"
#include "linker.h"
#include "parsing/syntax.h"
#include "tool.h"
#include "tools/stringlist.h"
#include "tools/tools.h"

#include <logger/log.h>
#include <process/stupidprocess.h>

#include <memory>
#include <string>
#include <thread>

using namespace std::chrono_literals;

void Processor::schedule(const Command &command)
{
    Log::information("Scheduling command: ", command.whole());

    const auto typeString = Syntax::commandString(command.type);

    std::unique_ptr<Process> process = nullptr;

    switch (command.type)
    {
    case Syntax::Command::Executable:
    case Syntax::Command::Library:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not implemented yet!");
        {
            Linker tool;
            tool.setup(command);

            if (not isDryRun())
            {
                process = std::make_unique<StupidProcess>();
                process->setExecutable(tool.command());
                process->setArguments(tool.arguments());
                process->setMetaInformation(std::to_string(command.id()) + " " +
                                            Syntax::commandString(command.type));
            }
        }
        break;
    case Syntax::Command::Option:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not implemented yet!");
        break;
    case Syntax::Command::Qt:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not implemented yet!");
        break;
    case Syntax::Command::Source:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not fully functional yet!");
        {
            Compiler tool;
            tool.setup(command);

            if (not isDryRun())
            {
                process = std::make_unique<StupidProcess>();
                process->setExecutable(tool.command());
                process->setArguments(tool.arguments());
                process->setMetaInformation(std::to_string(command.id()) + " " +
                                            Syntax::commandString(command.type));
            }
        }

        break;
    case Syntax::Command::Tool:
        Log::debug("Processing:", typeString, "command:", command.whole());
        break;
    case Syntax::Command::Include:
    case Syntax::Command::Feature:
    case Syntax::Command::Subproject:
    case Syntax::Command::Define:
    case Syntax::Command::Invalid:
    case Syntax::Command::Unknown:
        Log::warning("This command type:", typeString, "does not need to be processed");
        return;
    }

    if (process and not isDryRun())
    {
        process->setLogProcessOutput(isLogProcessOutput());
        _processes.push_back({command.id(), std::move(process)});
        _processes.back().process->start();
    }
}

void Processor::waitForFinished()
{
    forever
    {
        checkProcessStates();

        if (_processes.empty())
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

void Processor::checkProcessStates()
{
    for (size_t index = 0; index < _processes.size(); /* nothing */)
    {
        auto &current = _processes.at(index);

        if (current.process->isFinished())
        {
            _processes.erase(_processes.begin() + static_cast<long>(index));
        }
        else
        {
            ++index;
        }
    }
}
