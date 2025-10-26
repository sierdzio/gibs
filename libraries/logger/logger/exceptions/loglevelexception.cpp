#include "loglevelexception.h"
#include "logger/log.h"

#include <string>

LogLevelException::LogLevelException(const size_t value)
    : std::exception(),
      message("Invalid log level: " + std::to_string(value) + " is out of range 0-" +
              std::to_string(Log::logLevelsCount()))
{
}

const char *LogLevelException::what() const noexcept
{
    return message.c_str();
}
