#include "process.h"

#include <logger/log.h>

#include <unistd.h>

namespace
{
constexpr auto Space = " ";
} // namespace

Process::~Process()
{
    if (_thread.joinable())
    {
        _thread.join();
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

bool Process::start()
{
    Log::debug("Running process:", executable());

    _thread = std::thread(&Process::performWork, this);
    _thread.detach();

    Log::debug("Process", executable(), "started");

    return true;
}

bool Process::isFinished() const
{
    return _result.status != Exit::Status::InProgress and
           _result.status != Exit::Status::NotExecuted;
}

Exit Process::result() const
{
    return _result;
}

void Process::finish(const int code, const Exit::Status status)
{
    _result.rawCode = code;
    _result.status = status;

    if (_thread.joinable())
    {
        _thread.join();
    }
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
