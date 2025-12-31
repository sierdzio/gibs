#include "process.h"

#include <logger/log.h>
#include <string>

uint Process::_globalIdentifier = 1;

Process::Process() : _identifier(_globalIdentifier++)
{
}

Process::~Process()
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void Process::setExecutable(const std::string &filePath)
{
    std::lock_guard lock(_mutex);
    _executablePath = filePath;
}

const std::string &Process::executable() const
{
    return _executablePath;
}

void Process::setArguments(const Arguments &args)
{
    std::lock_guard lock(_mutex);
    _arguments = args;
}

const Arguments &Process::arguments() const
{
    return _arguments;
}

void Process::setMetaInformation(const std::string &information)
{
    std::lock_guard lock(_mutex);
    _metaInformation = information;
}

const std::string &Process::metaInformation() const
{
    return _metaInformation;
}

void Process::setLogProcessOutput(const bool enabled)
{
    _logProcessOutput = enabled;
}

bool Process::isLogProcessOutput() const
{
    return _logProcessOutput;
}

bool Process::start()
{
    if (executable().empty())
    {
        Log::error(logIdentifier(), "Cannot run process when executable name is empty!");
        finish(1, Exit::Status::FailedToExecute);
        return false;
    }

    Log::debug(Log::Color(Log::Standard::Foreground::Green),
                     logIdentifier(), " -> Running process:",
                     fullCommandLineCall(), "Extra info:", logMeta());

    _result.status = Exit::Status::InProgress;
    _thread = std::thread(&Process::performWork, this);
    _thread.detach();

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
    {
        std::lock_guard lock(_mutex);
        _result.rawCode = code;
        _result.status = status;
    }

    if (_thread.joinable())
    {
        _thread.join();
    }

    Log::debug(logIdentifier(), " -> Process has finished:",
               fullCommandLineCall(), "with exit code:", code);
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

std::string Process::fullCommandLineCall() const
{
    if (executable().empty() or arguments().empty())
    {
        return  {};
    }

    return executable() + ' ' + argsToString(arguments());
}

bool Process::hasMeta() const
{
    return not _metaInformation.empty();
}

std::string Process::logMeta() const
{
    return hasMeta() ? ("Meta: " + metaInformation()) : std::string();
}

uint Process::identifier() const
{
    return _identifier;
}

std::string Process::logIdentifier() const
{
    return '(' + std::to_string(identifier()) + ')';
}
