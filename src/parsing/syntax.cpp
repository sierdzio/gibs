#include "syntax.h"
#include "exceptions/commandexception.h"

#include <algorithm>
#include <array>

namespace
{
#define X(key, name) name,
constexpr std::array commandStrings = {COMMANDS};
#undef X
} //namespace

const std::string Syntax::commandString(const Syntax::Command command)
{
    const auto index = static_cast<size_t>(command);

    if (index >= commandStrings.size())
    {
        throw CommandException(index);
    }

    return commandStrings.at(index);
}

Syntax::Command Syntax::commandValue(const std::string &string)
{
    const auto it = std::find(commandStrings.cbegin(), commandStrings.cend(), string);

    if (it == commandStrings.cend())
    {
        throw CommandStringException(string);
    }

    return static_cast<Command>(std::distance(commandStrings.cbegin(), it));
}

size_t Syntax::commandCount()
{
    return commandStrings.size();
}
