#include "command.h"

#include "parsing/syntax.h"
#include "tools/log.h"
#include "tools/tools.h"
#include <filesystem>

namespace
{
    static CommandId uniqueId = 1;

    static CommandId nextId()
    {
        return uniqueId++;
    }

    constexpr auto Space = " ";
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
    if (modifiers.empty()) {
        return;
    }

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
                else if (command == Syntax::Command::Library)
                {
                    if (previous == Syntax::Modifier::Type)
                    {
                        if (current == Syntax::Modifier::Dynamic)
                        {
                            library.type = Syntax::LibraryType::Dynamic;
                        }
                        else if (current == Syntax::Modifier::Static)
                        {
                            library.type = Syntax::LibraryType::Static;
                        }
                        else
                        {
                            Log::warning("Unknown library type:", current);
                        }

                        previous.clear();
                        continue;
                    }
                    else
                    {
                        library.name = current;
                        previous.clear();
                        continue;
                    }
                }
                else if (command == Syntax::Command::Include)
                {
                    if (previous == Syntax::Modifier::Library)
                    {
                        include.isLibrary = true;
                        previous.clear();
                        continue;
                    }

                    include.path = current;
                }
                else if (command == Syntax::Command::Feature or command == Syntax::Command::Option)
                {
                    if (previous == Syntax::Modifier::Default)
                    {
                        if (current == Syntax::Modifier::On)
                        {
                            option.defaultValue = true;
                        }
                        else if (current == Syntax::Modifier::Off)
                        {
                            option.defaultValue = false;
                        }
                        else
                        {
                            Log::warning("Unrecognised default value:", current,
                                "for option:", Syntax::commandString(command));
                        }

                        previous.clear();
                        continue;
                    }

                    option.name = current;

                    previous.clear();
                    continue;
                }
            }

            previous = current;
        }
    } else {
        if (command == Syntax::Command::Source)
        {
            std::filesystem::path filePath = modifiers.back();
            filePath.replace_extension(Syntax::Extension::ObjectFile1);
            // TODO: use different extension per platform!
            Log::verbose("Appending object file:", filePath.string());
            object.name = filePath.string();
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
        mods.append(Space);
        mods.append(current);
    }

    std::string extra;

    if (Log::isWithinLogLevel(Log::Type::Verbose))
    {
        switch (command)
        {
            case Syntax::Command::Executable:
                extra = Space + Tools::inBrackets(Tools::listToString(executable.objects));
                break;
            case Syntax::Command::Library:
                extra = Space + Tools::inBrackets(Tools::listToString(library.objects));
                break;
            case Syntax::Command::Source:
                extra = Space + Tools::inBrackets(object.name);
                break;
            case Syntax::Command::Define:
            case Syntax::Command::Include:
            case Syntax::Command::Feature:
            case Syntax::Command::Option:
            case Syntax::Command::Subproject:
            case Syntax::Command::Tool:
            case Syntax::Command::Qt:
            case Syntax::Command::Invalid:
                break;
        }
    }

    return Syntax::commandString(command) + mods + extra;
}

std::string Command::value() const
{
    return modifiers.back();
}

bool Command::isValidCommand(const std::string &command) const
{
    return command == Syntax::commandString(Syntax::Command::Source)
        || command == Syntax::commandString(Syntax::Command::Library)
        || command == Syntax::commandString(Syntax::Command::Define)
        || command == Syntax::commandString(Syntax::Command::Include)
        || command == Syntax::commandString(Syntax::Command::Executable);
}

bool Command::supportsModifiers(const Syntax::Command command) const
{
    switch (command) {
    case Syntax::Command::Library:
    case Syntax::Command::Executable:
    case Syntax::Command::Include:
    case Syntax::Command::Feature:
    case Syntax::Command::Option:
        return true;
    case Syntax::Command::Invalid:
    case Syntax::Command::Define:
    case Syntax::Command::Source:
    case Syntax::Command::Subproject:
    case Syntax::Command::Tool:
    case Syntax::Command::Qt:
        return false;
    }

    return false;
}
