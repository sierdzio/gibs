#include "stupidprocess.h"
#include "processinterface.h"

#include <logger/log.h>

#include <filesystem>
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
    if (duration() > std::chrono::milliseconds(0))
    {
        const auto fullInvocation = fullCommandLineCall();

        for (const auto i : std::views::iota(1, 6))
        {
            if (isLogProcessOutput())
            {
                Log::verbose(logIdentifier(), " -> Process:", fullInvocation,
                             "iteration:", i);
            }
            std::this_thread::sleep_for(_duration);
        }
    }

    finish(0, Exit::Status::Success);
}
