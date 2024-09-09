#include "log.h"

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
    constexpr auto Debug = "D:";
    constexpr auto Information = "I:";
    constexpr auto Warning = "W:";
    constexpr auto Error = "E:";
}

std::string Log::type(const Type type)
{
    switch (type)
    {
        case Type::Debug:
            return Debug;
        case Type::Information:
            return Information;
        case Type::Warning:
            return Warning;
        case Type::Error:
            return Error;
    }

    return {};
}
