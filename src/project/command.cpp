#include "command.h"
#include "exceptions/commandexception.h"
#include "exceptions/emptylinkobject.h"
#include "parsing/syntax.h"
#include "tools/log.h"
#include "tools/tools.h"

#include <coroutine>
#include <filesystem>

namespace
{
static CommandId uniqueId = 1;

static CommandId nextId()
{
    return uniqueId++;
}

constexpr auto Space = " ";
} // namespace

Command::Command() : _id(nextId())
{
}

uint Command::id() const
{
    return _id;
}

bool Command::isValid() const
{
    if (parsingFailed or type == Syntax::Command::Invalid)
    {
        return false;
    }

    return true;
}

bool Command::append(const std::string &part)
{
    if (type == Syntax::Command::Invalid)
    {
        if (part == Syntax::CppKeywords::Include)
        {
            type = Syntax::Command::Include;
            return true;
        }
        else if (isValidCommand(part))
        {
            type = Syntax::commandValue(part);
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
        if (not supportsModifiers(type) and not modifiers.empty())
        {
            Log::warning("Got another command value:", part,
                         "but a previous one already exists:", value());
            parsingFailed = true;
            return false;
        }

        modifiers.emplace_back(Tools::prepareIncludePath(std::move(part)));
        return true;
    }

    parsingFailed = true;
    return false;
}

bool Command::hasModifiers() const
{
    return not modifiers.empty();
}

void Command::finalize()
{
    if (modifiers.empty())
    {
        return;
    }

    if (supportsModifiers(type))
    {
        std::string previous;
        for (const auto &current : modifiers)
        {
            if (type == Syntax::Command::Executable && previous == Syntax::Modifier::Name)
            {
                executable.name = current;
                previous.clear();
                continue;
            }
            else if (type == Syntax::Command::Library)
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
                else if (previous == Syntax::Modifier::Name)
                {
                    library.name = current;
                    previous.clear();
                    continue;
                }
                else
                {
                    Log::warning("Unknown library modifier:", previous, current);
                    continue;
                }
            }
            else if (type == Syntax::Command::Include)
            {
                if (previous == Syntax::Modifier::Library)
                {
                    include.isLibrary = true;
                    include.path = current;
                    previous.clear();
                    continue;
                }

                include.path = Tools::prepareIncludePath(current);
            }
            else if (type == Syntax::Command::Feature or type == Syntax::Command::Option)
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
                                     "for option:", Syntax::commandString(type));
                    }

                    previous.clear();
                    continue;
                }

                option.name = current;

                previous.clear();
                continue;
            }

            previous = current;
        }

        // All modifiers parsed. Final adjustments:
        if (type == Syntax::Command::Library)
        {
            std::filesystem::path filePath = library.name;
            if (library.type == Syntax::LibraryType::Static)
            {
                filePath.replace_extension(Syntax::Extension::LibraryStatic);
            }
            else if (library.type == Syntax::LibraryType::Dynamic)
            {
                filePath.replace_extension(Syntax::Extension::LibraryDynamic);
            }
            // TODO: use different extension per platform!
            Log::verbose("Appending library file:", filePath.string());
            object.name = filePath.string();
        }
    }
    else
    {
        if (type == Syntax::Command::Source)
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

    for (const auto &current : modifiers)
    {
        mods.append(Space);
        mods.append(current);
    }

    std::string extra;

    if (Log::isWithinLogLevel(Log::Type::Verbose))
    {
        switch (type)
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
        case Syntax::Command::Include:
            if (include.isLibrary)
                extra = Space + Tools::inBrackets(
                                    Syntax::commandString(Syntax::Command::Library));
            break;
        case Syntax::Command::Feature:
        case Syntax::Command::Option:
            extra =
                Space + Tools::inBrackets(std::string(Syntax::Modifier::Default) + Space +
                                          Tools::boolToString(option.defaultValue));
            break;
        case Syntax::Command::Define:
        case Syntax::Command::Subproject:
        case Syntax::Command::Tool:
        case Syntax::Command::Qt:
        case Syntax::Command::Invalid:
            break;
        }
    }

    return Syntax::commandString(type) + mods + extra;
}

std::string Command::value() const
{
    if (modifiers.empty())
    {
        return {};
    }

    return modifiers.back();
}

std::string Command::path() const
{
    if (type == Syntax::Command::Include)
    {
        return include.path;
    }
    else if (type == Syntax::Command::Source)
    {
        return object.name;
    }
    else if (type == Syntax::Command::Executable)
    {
        return executable.name;
    }
    else if (type == Syntax::Command::Library)
    {
        return library.name;
    }

    switch (type)
    {
    case Syntax::Command::Include:
        return include.path;
    case Syntax::Command::Source:
        return object.name;
    case Syntax::Command::Executable:
        return executable.name;
    case Syntax::Command::Library:
        return library.name;
    case Syntax::Command::Define:
    case Syntax::Command::Feature:
    case Syntax::Command::Option:
    case Syntax::Command::Qt:
    case Syntax::Command::Subproject:
    case Syntax::Command::Tool:
    case Syntax::Command::Invalid:
        break;
    }

    Log::error("Path requested from command which does not support it:",
               Syntax::commandString(type), "available modifiers are:", modifiers);

    return {};
}

bool Command::addLinkObject(const std::string &name)
{
    if (name.empty())
    {
        throw EmptyLinkObject(*this);
    }

    if (type == Syntax::Command::Executable)
    {
        executable.objects.push_back(name);
    }
    else if (type == Syntax::Command::Library)
    {
        library.objects.push_back(name);
    }
    else
    {
        Log::verbose(
            "Tried to add link object", name,
            "to a command which is neither a library nor an executable:", whole());
        return false;
    }

    return true;
}

bool Command::isValidCommand(const std::string &command) const
{
    try
    {
        Syntax::commandValue(command);
        return true;
    }
    catch (const CommandStringException &e)
    {
        Log::verbose(e.what());
        return false;
    }
}

bool Command::supportsModifiers(const Syntax::Command command) const
{
    switch (command)
    {
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
