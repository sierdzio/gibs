#include "syntax.h"

Syntax::Command Syntax::commandValue(const std::string &string)
    {
        const auto it = std::find(commandString.cbegin(), commandString.cend(), string);

        if (it == commandString.cend()) {
            return Command::Invalid;
        }

        return static_cast<Command>(std::distance(commandString.cbegin(), it));
    }
