#include "processor.h"
#include "parsing/syntax.h"
#include "tools/log.h"

#include <process/stupidprocess.h>

Processor::Processor()
{
}

void Processor::schedule(const Command &command)
{
    Log::information("Scheduling command: ", command.whole());

    const auto typeString = Syntax::commandString(command.type);

    switch (command.type)
    {
    case Syntax::Command::Executable:
    case Syntax::Command::Library:
    case Syntax::Command::Option:
    case Syntax::Command::Qt:
    case Syntax::Command::Source:
    case Syntax::Command::Tool:
        Log::debug("Processing:", typeString);
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
}
