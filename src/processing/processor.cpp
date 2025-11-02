#include "processor.h"
#include "parsing/syntax.h"
#include "tools/stringlist.h"
#include "tools/tools.h"

#include <logger/log.h>
#include <process/stupidprocess.h>

#include <memory>
#include <thread>

using namespace std::chrono_literals;

struct Tool
{
    virtual bool setup(const Command &command) = 0;
    virtual std::string command() const = 0;
    virtual StringList arguments() const = 0;
};

struct Compiler : public Tool
{
    bool setup(const Command &command) override
    {
        if (command.object().name.empty())
        {
            Log::error("Cannot compile: no source file path!");
            return false;
        }

        for (const auto &current : command.object().includePaths)
        {
            if (current.empty())
            {
                continue;
            }

            _arguments.push_back("-I");
            _arguments.push_back(current);
        }

        _arguments.push_back(command.object().name);

        return true;
    }

    std::string command() const override
    {
        return "g++";
    }

    StringList arguments() const override
    {
        return _arguments;
    }

  private:
    StringList _arguments;
};

struct Linker : public Tool
{
    bool setup(const Command &command) override
    {
        const bool isExe = command.type == Syntax::Command::Executable;

        const auto &objects =
            isExe ? command.executable().objects : command.library().objects;

        for (const auto &current : objects)
        {
            _arguments.push_back(current);
        }

        if (not isExe)
        {
            _arguments.push_back(command.library().type == Syntax::LibraryType::Dynamic
                                     ? "-shared"
                                     : "-static");
        }

        _arguments.push_back("-o");
        _arguments.push_back(isExe ? command.executable().name : command.library().name);

        return true;
    }

    std::string command() const override
    {
        return "g++";
    }

    StringList arguments() const override
    {
        return _arguments;
    }

  private:
    StringList _arguments;
};

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
        _processes.push_back({command.id(), std::move(process)});
        _processes.back().process->start();
    }
}

void Processor::waitForFinished()
{
    forever
    {
        Log::debug("Checking processes, count:", _processes.size());

        checkProcessStates();

        if (_processes.empty())
        {
            Log::debug("All processes have finished.");
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

void Processor::checkProcessStates()
{
    for (size_t index = 0; index < _processes.size(); /* nothing */)
    {
        auto &current = _processes.at(index);

        if (current.process->isFinished())
        {
            Log::information("Process finished:", current.process->executable(),
                             "command ID:", current.commandId);
            _processes.erase(_processes.begin() + static_cast<long>(index));
        }
        else
        {
            ++index;
        }
    }
}
