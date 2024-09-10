#include "command.h"
#include "syntax.h"

#include "tools/log.h"

bool Command::isValid() const
{
    if (command == Syntax::Command::Invalid) {
        return false;
    }

    // TODO: check modifiers
    // || first == Syntax::Command::Type
    // || first == Syntax::Command::App
    // || first == Syntax::Command::Static
    // || first == Syntax::Command::Dynamic

    return true;
}

bool Command::append(const std::string &part)
{
    if (command == Syntax::Command::Invalid) {
        if (isValidCommand(part)) {
            command = Syntax::commandValue(part);
        } else {
            Log::warning("Invalid project command:", part);
            return false;
        }
    }

    return false;
}

std::string Command::whole() const
{
    std::string mods;

    for (const auto &current : modifiers) {
        mods.append(" ");
        mods.append(current);
    }

    return Syntax::commandString[command] + mods + value;
}

bool Command::isValidCommand(const std::string &command) const
{
    return command == Syntax::commandString[Syntax::Command::Source]
        || command == Syntax::commandString[Syntax::Command::Target]
        || command == Syntax::commandString[Syntax::Command::Lib]
        || command == Syntax::commandString[Syntax::Command::Define]
        || command == Syntax::commandString[Syntax::Command::Include];
}
