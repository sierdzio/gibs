#include "log.h"

#include <algorithm>

std::ostream &operator<<(std::ostream &stream, const std::vector<std::string> &stringList)
{
    for (std::size_t i = 0; i < stringList.size(); ++i) {
        if (i != 0) {
            stream << ' ';
        }

        stream << stringList.at(i);
    }

    return stream;
}

namespace {
    constexpr auto Verbose = "V:";
    constexpr auto Debug = "D:";
    constexpr auto Information = "I:";
    constexpr auto Warning = "W:";
    constexpr auto Error = "E:";
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

const std::string Log::typeString(const Log::Type type)
{
    return typeStrings.at(static_cast<size_t>(type));
}

Log::Type Log::typeValue(const std::string &string)
{
    // TODO: make it case-insensitive
    const auto it = std::find(typeStrings.cbegin(), typeStrings.cend(), string);

    if (it == typeStrings.cend()) {
        return Type::Information;
    }

    return static_cast<Type>(std::distance(typeStrings.cbegin(), it));
}
