#include "process.h"

#include <logger/log.h>

#include <thread>
#include <unistd.h>

namespace
{
constexpr auto Space = " ";

std::thread thread;

} // namespace

Process::~Process()
{
    if (thread.joinable())
    {
        thread.join();
    }
}

void Process::setExecutable(const std::string &filePath)
{
    _executablePath = filePath;
}

std::string Process::executable() const
{
    return _executablePath;
}

void Process::setArguments(const Arguments &args)
{
    _arguments = args;
}

Arguments Process::arguments() const
{
    return _arguments;
}

bool Process::execute()
{
    Log::debug("Running process:", executable());

    thread = std::thread(&Process::start, this);
    thread.detach();

    Log::debug("Process", executable(), "started");

    return true;
}

Exit Process::result() const
{
    return _result;
}

std::string Process::argsToString(const Arguments &args) const
{
    std::string result;

    for (const auto &arg : args)
    {
        if (not result.empty())
        {
            result.append(Space);
        }

        result.append(arg);
    }

    return result;
}
