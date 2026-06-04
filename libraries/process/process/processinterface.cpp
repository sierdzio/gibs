#include "processinterface.h"

#include <logger/log.h>
#include <string>

unsigned int ProcessInterface::_globalIdentifier = 1;

ProcessInterface::ProcessInterface() : _identifier(_globalIdentifier++)
{
}

ProcessInterface::~ProcessInterface()
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void ProcessInterface::setExecutable(const std::string &filePath)
{
    std::lock_guard lock(_mutex);
    _executablePath = filePath;
}

const std::string &ProcessInterface::executable() const
{
    return _executablePath;
}

void ProcessInterface::setArguments(const Arguments &args)
{
    std::lock_guard lock(_mutex);
    _arguments = args;
}

const Arguments &ProcessInterface::arguments() const
{
    return _arguments;
}

void ProcessInterface::setMetaInformation(const std::string &information)
{
    std::lock_guard lock(_mutex);
    _metaInformation = information;
}

const std::string &ProcessInterface::metaInformation() const
{
    return _metaInformation;
}

void ProcessInterface::setLogProcessOutput(const bool enabled)
{
    _logProcessOutput = enabled;
}

bool ProcessInterface::isLogProcessOutput() const
{
    return _logProcessOutput;
}

bool ProcessInterface::start()
{
    if (executable().empty())
    {
        Log::error(logIdentifier(), "Cannot run process when executable name is empty!");
        finish(1, Exit::Status::FailedToExecute);
        return false;
    }

    Log::debug(Log::Color(Log::Standard::Foreground::Green), logIdentifier(),
               " -> Running process:", fullCommandLineCall(), "Extra info:", logMeta());

    _result.status = Exit::Status::InProgress;
    _thread = std::thread(&ProcessInterface::performWork, this);
    _thread.detach();

    return true;
}

bool ProcessInterface::isFinished() const
{
    std::lock_guard lock(_mutex);
    return _result.status != Exit::Status::InProgress and
           _result.status != Exit::Status::NotExecuted;
}

Exit ProcessInterface::result() const
{
    std::lock_guard lock(_mutex);
    return _result;
}

std::string ProcessInterface::fullCommandLineCall() const
{
    if (executable().empty() or arguments().empty())
    {
        return {};
    }

    return executable() + ' ' + argsToString(arguments());
}

void ProcessInterface::finish(const int code, const Exit::Status status)
{
    const auto logId = logIdentifier();
    const auto commandLine = fullCommandLineCall();

    // Update result under lock. Do not attempt to join the thread here — joining
    // from within the worker may attempt to join the current thread and throw
    // std::system_error. Thread lifetime is managed by either detaching (in
    // start()) or joining in the destructor when appropriate.
    {
        std::lock_guard lock(_mutex);
        _result.rawCode = code;
        _result.status = status;
    }

    Log::debug(logId, " -> Process has finished:", commandLine, "with exit code:", code);
}

std::string ProcessInterface::argsToString(const Arguments &args) const
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

bool ProcessInterface::hasMeta() const
{
    return not _metaInformation.empty();
}

std::string ProcessInterface::logMeta() const
{
    return hasMeta() ? ("Meta: " + metaInformation()) : std::string();
}

unsigned int ProcessInterface::identifier() const
{
    return _identifier;
}

std::string ProcessInterface::logIdentifier() const
{
    return '(' + std::to_string(identifier()) + ')';
}
