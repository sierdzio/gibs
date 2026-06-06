#include "log.h"
#include "exceptions/loglevelexception.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iosfwd>

static Log::Type RuntimeLogLevel = Log::Type::Verbose;
static bool UseColors = true;
static std::ofstream LogFileStream;

namespace
{
constexpr auto Nl = '\n';
constexpr auto ColorEnd = "\033[0m";

constexpr auto Verbose = "V:";
constexpr auto Debug = "D:";
constexpr auto Information = "I:";
constexpr auto Warning = "W:";
constexpr auto Error = "E:";

#define X(key, name) name,
constexpr std::array TypeStrings = {LOG_TYPES};
#undef X

void checkBounds(const Log::Type type)
{
    const auto raw = static_cast<size_t>(type);

    if (raw >= TypeStrings.size())
    {
        throw LogLevelException(raw);
    }
}

bool hasColor(const Log::Type type, const Log::Color &color)
{
    if (not UseColors) [[unlikely]]
    {
        return false;
    }

    if (not color.isDefault())
    {
        return true;
    }

    checkBounds(type);

    switch (type)
    {
    case Log::Type::Error:
    case Log::Type::Warning:
    case Log::Type::Information:
        return true;
    case Log::Type::Debug:
    case Log::Type::Verbose:
    case Log::Type::Silent:
        return {};
    }

    return {};
}

std::string typeToPrint(const Log::Type type)
{
    checkBounds(type);

    switch (type)
    {
    case Log::Type::Verbose:
        return Verbose;
    case Log::Type::Debug:
        return Debug;
    case Log::Type::Information:
        return Information;
    case Log::Type::Warning:
        return Warning;
    case Log::Type::Error:
        return Error;
    case Log::Type::Silent:
        return {};
    }

    return {};
}

std::string typeColor(const Log::Type type, const Log::Color &color)
{
    if (not color.isDefault())
    {
        return color.ansiEscapeCode();
    }

    checkBounds(type);

    switch (type)
    {
    case Log::Type::Error:
        return Log::Color(Log::Standard::Foreground::Red).ansiEscapeCode();
    case Log::Type::Warning:
        return Log::Color(Log::Standard::Foreground::Yellow).ansiEscapeCode();
    case Log::Type::Information:
        return Log::Color(Log::Standard::Foreground::Blue).ansiEscapeCode();
    case Log::Type::Debug:
        return Log::Color(Log::Standard::Foreground::BrightBlack).ansiEscapeCode();
    case Log::Type::Verbose:
    case Log::Type::Silent:
        return {};
    }

    return {};
}

} //namespace

std::ostream &operator<<(std::ostream &stream, const std::vector<std::string> &stringList)
{
    for (std::size_t i = 0; i < stringList.size(); ++i)
    {
        if (i != 0) [[likely]]
        {
            stream << ' ';
        }

        stream << stringList.at(i);
    }

    return stream;
}

const std::string Log::typeString(const Log::Type type)
{
    checkBounds(type);

    const auto raw = static_cast<size_t>(type);
    return TypeStrings.at(raw);
}

Log::Type Log::typeValue(const std::string &string)
{
    // TODO: make it case-insensitive
    const auto it = std::find(TypeStrings.cbegin(), TypeStrings.cend(), string);

    if (it == TypeStrings.cend())
    {
        return Type::Information;
    }

    return static_cast<Type>(std::distance(TypeStrings.cbegin(), it));
}

void Log::setLogLevel(const Type type)
{
    const auto raw = static_cast<size_t>(type);

    if (raw >= TypeStrings.size())
    {
        throw LogLevelException(raw);
    }

    RuntimeLogLevel = type;
}

Log::Type Log::logLevel()
{
    return RuntimeLogLevel;
}

size_t Log::logLevelsCount()
{
    return TypeStrings.size();
}

bool Log::isWithinLogLevel(const Type type)
{
    checkBounds(type);
    return static_cast<int>(type) <= static_cast<int>(RuntimeLogLevel);
}

void Log::setUseColorfulLogs(const bool enableColor)
{
    UseColors = enableColor;
}

bool Log::usingColorfulLogs()
{
    return UseColors;
}

void Log::setLogFile(const std::string &path)
{
    if (LogFileStream.is_open())
    {
        LogFileStream.close();
    }

    LogFileStream.open(path, std::ios::out | std::ios::trunc);

    if (not LogFileStream.is_open())
    {
        throw std::runtime_error("Cannot open log file: " + path);
    }
}

void Log::closeLogFile()
{
    if (LogFileStream.is_open())
    {
        LogFileStream.close();
    }
}

bool Log::Private::isFileLoggingEnabled()
{
    return LogFileStream.is_open();
}

std::ostream &Log::Private::logFileStream()
{
    return LogFileStream;
}

std::string Log::Private::beginning(const Type type, const Color &color)
{
    return typeColor(type, color) + typeToPrint(type) + ' ';
}

std::string Log::Private::ending(const Type type, const Color &color)
{
    return (hasColor(type, color) ? ColorEnd : std::string()) + Nl;
}

std::ostream &Log::operator<<(std::ostream &stream, const Type type)
{
    stream << Log::typeString(type);
    return stream;
}
