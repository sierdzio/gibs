#include "stupidprocess.h"
#include "process/process.h"

#include <logger/log.h>

#include <ranges>
#include <thread>

void StupidProcess::setDuration(const std::chrono::milliseconds duration)
{
    _duration = duration;
}

std::chrono::milliseconds StupidProcess::duration() const
{
    return _duration;
}

void StupidProcess::performWork()
{
    _result.status = Exit::Status::InProgress;

    Log::debug(logIdentifier(), " -> Executing process:", fullCommandLineCall(),
               "with args:", argsToString(arguments()));

    for (const auto i : std::views::iota(1, 6))
    {
        Log::debug(logIdentifier(), " -> Process:", fullCommandLineCall(), "iteration:", i);
        std::this_thread::sleep_for(_duration);
    }

    Log::debug(logIdentifier(), " -> Done! Process has finished:", fullCommandLineCall());

    finish(0, Exit::Status::Success);
}
