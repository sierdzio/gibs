#include "processor.h"
#include "tools/log.h"

Processor::Processor()
{
}

void Processor::schedule(const Command &command)
{
    Log::information("Scheduling command: ", command.whole());
}
