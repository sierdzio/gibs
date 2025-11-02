#include "log.h"
#include "exceptions/loglevelexception.h"

#include <algorithm>
#include <array>
#include <iosfwd>

static Log::Type RuntimeLogLevel = Log::Type::Information;
static bool UseColors = true;

namespace
{
constexpr auto Space = ' ';
constexpr auto Nl = '\n';
constexpr auto Red = "\033[31m";
constexpr auto Yellow = "\033[33m";
constexpr auto Blue = "\033[34m";
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

std::string typeColor(const Log::Type type)
{
    checkBounds(type);

    switch (type)
    {
    case Log::Type::Error:
        return Red;
    case Log::Type::Warning:
        return Yellow;
    case Log::Type::Information:
        return Blue;
    case Log::Type::Debug:
    case Log::Type::Verbose:
    case Log::Type::Silent:
        return {};
    }

    return {};
}

bool hasColor(const Log::Type type)
{
    if (not UseColors) [[unlikely]]
    {
        return false;
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

} //namespace

std::ostream &operator<<(std::ostream &stream, const std::vector<std::string> &stringList)
{
    for (std::size_t i = 0; i < stringList.size(); ++i)
    {
        if (i != 0) [[likely]]
        {
            stream << Space;
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

std::string Log::beginning(const Type type)
{
    return typeColor(type) + typeToPrint(type) + Space;
}

std::string Log::ending(const Type type)
{
    return (hasColor(type) ? ColorEnd : std::string()) + Nl;
}
