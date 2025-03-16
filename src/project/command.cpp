#include "command.h"

#include "parsing/syntax.h"
#include "tools/log.h"
#include "tools/tools.h"

namespace
{
    static CommandId uniqueId = 1;

    static CommandId nextId()
    {
        return uniqueId++;
    }
}

bool ExecutableComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Executable && name.size() > 0;
}

bool LibraryComponent::isValid(const Syntax::Command type) const
{
    return type == Syntax::Command::Lib && name.size() > 0;
}

Command::Command() : _id(nextId())
{
}

uint Command::id() const
{
    return _id;
}

bool Command::isValid() const
{
    if (parsingFailed or command == Syntax::Command::Invalid) {
        return false;
    }

    return true;
}

bool Command::append(const std::string &part)
{
    if (command == Syntax::Command::Invalid)
    {
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
            parsingFailed = true;
            return false;
        }
    }
    else
    {
        if (not supportsModifiers(command) and not modifiers.empty())
        {
            Log::warning("Got another command value:", part,
                "but a previous one already exists:", value());
                parsingFailed = true;
            return false;
        }

        modifiers.push_back(Tools::removeQuotes(std::move(part)));
        return true;
    }

    parsingFailed = true;
    return false;
}

void Command::finalize()
{
    if (supportsModifiers(command))
    {
        std::string previous;
        for (const auto& current : modifiers)
        {
            if (not previous.empty())
            {
                if (command == Syntax::Command::Executable && previous == Syntax::Modifier::Name)
                {
                    executable.name = current;
                    previous.clear();
                    continue;
                }
                else if (command == Syntax::Command::Lib && previous == Syntax::Modifier::Name)
                {
                    library.name = current;
                    previous.clear();
                    continue;
                }
            }

            previous = current;
        }
    }
}

bool Command::isReadyToExecute() const
{
    return isReadyToExe;
}

void Command::setIsReadyToExecute(const bool ready)
{
    isReadyToExe = ready;
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
        || command == Syntax::commandString(Syntax::Command::Include)
        || command == Syntax::commandString(Syntax::Command::Executable);
}

bool Command::supportsModifiers(const Syntax::Command command) const
{
    switch (command) {
    case Syntax::Command::Lib:
    case Syntax::Command::Target:
    case Syntax::Command::Executable:
        return true;
    case Syntax::Command::Invalid:
    case Syntax::Command::Define:
    case Syntax::Command::Include:
    case Syntax::Command::Source:
        return false;
    }

    return false;
}
