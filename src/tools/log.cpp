#include "log.h"

#include <algorithm>
#include <array>

static Log::Type RuntimeLogLevel = Log::Type::Information;
static bool UseColors = true;

namespace
{
constexpr auto Space = ' ';
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
    return TypeStrings.at(static_cast<size_t>(type));
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
    RuntimeLogLevel = type;
}

bool Log::isWithinLogLevel(const Type type)
{
    return static_cast<int>(type) <= static_cast<int>(RuntimeLogLevel);
}

void Log::setUseColorfulLogs(const bool enableColor)
{
    UseColors = enableColor;
}

std::string Log::type(const Type type)
{
    switch (type)
    {
    case Type::Verbose:
        return Verbose;
    case Type::Debug:
        return Debug;
    case Type::Information:
        return Information;
    case Type::Warning:
        return Warning;
    case Type::Error:
        return Error;
    case Type::Silent:
        return {};
    }

    return {};
}

std::string Log::typeColor(const Type type)
{
    switch (type)
    {
    case Type::Error:
        return Red;
    case Type::Warning:
        return Yellow;
    case Type::Information:
        return Blue;
    case Type::Debug:
    case Type::Verbose:
    case Type::Silent:
        return {};
    }
}

bool isLoggingThisColor(const Log::Type type)
{
    if (not UseColors) [[unlikely]]
    {
        return false;
    }

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
}

std::string Log::beginning(const Type type)
{
    return (isLoggingThisColor(type) ? typeColor(type) : std::string()) +
           Log::type(type) + Space;
}

std::string Log::ending(const Type type)
{
    return (isLoggingThisColor(type) ? ColorEnd : std::string()) + '\n';
}
