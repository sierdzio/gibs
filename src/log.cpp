#include "log.h"

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
