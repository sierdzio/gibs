#include "stupidprocess.h"
#include "process/process.h"

#include <logger/log.h>

#include <ranges>
#include <thread>

using namespace std::chrono_literals;

void StupidProcess::performWork()
{
    _result.status = Exit::Status::InProgress;

    Log::debug("  -> Executing process:", executable(),
               "with args:", argsToString(arguments()));

    for (const auto i : std::views::iota(1, 6))
    {
        Log::debug("  -> Process:", executable(), "iteration:", i);
        std::this_thread::sleep_for(1s);
    }

    Log::debug("  -> Done! Process has finished:", executable());

    finish(0, Exit::Status::Success);
}