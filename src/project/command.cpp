#include "command.h"

#include "parsing/syntax.h"
#include "tools/log.h"
#include "tools/tools.h"

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
        if (part == Syntax::CppKeywords::Include)
        {
            command = Syntax::Command::Include;
            return true;
        }
        else if (isValidCommand(part))
        {
            command = Syntax::commandValue(part);
            return true;
        }
        else
        {
            Log::warning("Invalid project command:", part);
            return false;
        }
    }
    else
    {
        if (not supportsModifiers(command) and not modifiers.empty())
        {
            Log::warning("Got another command value:", part,
                "but a previous one already exists:", value());
            return false;
        }

        modifiers.push_back(Tools::removeQuotes(std::move(part)));
        return true;
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

    return Syntax::commandString(command) + mods;
}

std::string Command::value() const
{
    return modifiers.back();
}

bool Command::isValidCommand(const std::string &command) const
{
    return command == Syntax::commandString(Syntax::Command::Source)
        || command == Syntax::commandString(Syntax::Command::Target)
        || command == Syntax::commandString(Syntax::Command::Lib)
        || command == Syntax::commandString(Syntax::Command::Define)
        || command == Syntax::commandString(Syntax::Command::Include);
}

bool Command::supportsModifiers(const Syntax::Command command) const
{
    switch (command) {
    case Syntax::Command::Lib:
    case Syntax::Command::Target:
        return true;
    case Syntax::Command::Invalid:
    case Syntax::Command::Define:
    case Syntax::Command::Include:
    case Syntax::Command::Source:
        return false;
    }

    return false;
}
