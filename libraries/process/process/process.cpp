#include "process.h"

#include <logger/log.h>
#include <string>

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

const std::string &Process::executable() const
{
    return _executablePath;
}

void Process::setArguments(const Arguments &args)
{
    _arguments = args;
}

const Arguments &Process::arguments() const
{
    return _arguments;
}

void Process::setMetaInformation(const std::string &information)
{
    _metaInformation = information;
}

const std::string &Process::metaInformation() const
{
    return _metaInformation;
}

bool Process::start()
{
    Log::information(Log::Color(Log::Standard::Foreground::Green),
                     "Running process:", executable(), arguments(), logMeta());

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
            result.push_back(' ');
        }

        result.append(arg);
    }

    return result;
}

bool Process::hasMeta() const
{
    return not _metaInformation.empty();
}

std::string Process::logMeta() const
{
    return hasMeta() ? ("Meta: " + metaInformation()) : std::string();
}
