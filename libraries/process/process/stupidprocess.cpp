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

    for (const auto i : std::views::iota(1, 6))
    {
        Log::verbose(logIdentifier(), " -> Process:", fullCommandLineCall(), "iteration:", i);
        std::this_thread::sleep_for(_duration);
    }

    finish(0, Exit::Status::Success);
}
