#include "syntax.h"

#include <algorithm>

const std::string Syntax::commandString(const Syntax::Command command)
{
    return commandStrings.at(static_cast<size_t>(command));
}

Syntax::Command Syntax::commandValue(const std::string &string)
    {
        const auto it = std::find(commandStrings.cbegin(), commandStrings.cend(), string);

        if (it == commandStrings.cend()) {
            return Command::Invalid;
        }

        return static_cast<Command>(std::distance(commandStrings.cbegin(), it));
    }
