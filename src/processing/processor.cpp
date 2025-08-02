#include "processor.h"
#include "parsing/syntax.h"
#include "tools/log.h"

#include <process/stupidprocess.h>

#include <thread>

Processor::Processor()
{
}

void Processor::schedule(const Command &command)
{
    Log::information("Scheduling command: ", command.whole());

    const auto typeString = Syntax::commandString(command.type);

    Process *process = nullptr;

    switch (command.type)
    {
    case Syntax::Command::Executable:
    case Syntax::Command::Library:
    case Syntax::Command::Option:
    case Syntax::Command::Qt:
    case Syntax::Command::Source:
    case Syntax::Command::Tool:
        if (_dryRun)
        {
            Log::information("Simulating:", typeString, "command:", command.whole());
        }
        else
        {
            Log::debug("Processing:", typeString, "command:", command.whole());
            // TODO: actual threading or future, or async, or something
            //std::thread thread;
            //thread.detach();

            process = new StupidProcess;
            // TODO: get real data from command:
            process->setExecutable("g++");
            process->setArguments({command.object().name});
            process->execute();
        }

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

    if (process)
    {
        processes.insert({command.id(), process});
    }
}

void Processor::setDryRun(const bool dryRun)
{
    _dryRun = dryRun;
}