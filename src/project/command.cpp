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
constexpr auto CommandNotValid = "Command is not valid:";
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
    if (_parsingFailed or type == Syntax::Command::Invalid)
    {
        Log::warning(CommandNotValid, "parsing has failed");
        return false;
    }

    if (type == Syntax::Command::Invalid)
    {
        Log::warning(CommandNotValid, "command type is not valid");
        return false;
    }

    if (_modifiers.empty())
    {
        Log::warning(
            CommandNotValid,
            "command requires a value and/ or modifiers but none have been provided");
        return false;
    }

    return true;
}

bool Command::append(const std::string &part)
{
    if (type == Syntax::Command::Unknown)
    {
        if (part == Syntax::CppKeywords::Include)
        {
            type = Syntax::Command::Include;
            return true;
        }
        else if (const auto result = getCommand(part); result.has_value())
        {
            type = result.value();
            return true;
        }
        else
        {
            Log::warning("Invalid project command:", part);
            type = Syntax::Command::Invalid;
            _parsingFailed = true;
            return false;
        }
    }
    else if (type == Syntax::Command::Invalid)
    {
        Log::error("Command has not been recognized, so adding further modifiers to it "
                   "will have no effect. Modifier:",
                   part);
        _parsingFailed = true;
        return false;
    }
    else
    {
        if (not supportsModifiers(type) and not _modifiers.empty())
        {
            Log::error("Got another command value:", part,
                       "but a previous one already exists:", value());
            _parsingFailed = true;
            return false;
        }

        _modifiers.emplace_back(Tools::prepareIncludePath(std::move(part)));

        // TODO: check isValid() for given command type

        return true;
    }

    _parsingFailed = true;
    return false;
}

bool Command::hasModifiers() const
{
    return not _modifiers.empty();
}

void Command::finalize()
{
    if (_modifiers.empty())
    {
        return;
    }

    if (supportsModifiers(type))
    {
        std::string previous;
        for (const auto &current : _modifiers)
        {
            if (type == Syntax::Command::Executable && previous == Syntax::Modifier::Name)
            {
                _executable.name = current;
                previous.clear();
                continue;
            }
            else if (type == Syntax::Command::Library)
            {
                if (previous == Syntax::Modifier::Type)
                {
                    if (current == Syntax::Modifier::Dynamic)
                    {
                        _library.type = Syntax::LibraryType::Dynamic;
                    }
                    else if (current == Syntax::Modifier::Static)
                    {
                        _library.type = Syntax::LibraryType::Static;
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
                    _library.name = current;
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
                    _include.isLibrary = true;
                    _include.path = current;
                    previous.clear();
                    continue;
                }

                _include.path = Tools::prepareIncludePath(current);
            }
            else if (type == Syntax::Command::Feature or type == Syntax::Command::Option)
            {
                if (previous == Syntax::Modifier::Default)
                {
                    if (current == Syntax::Modifier::On)
                    {
                        _option.defaultValue = true;
                    }
                    else if (current == Syntax::Modifier::Off)
                    {
                        _option.defaultValue = false;
                    }
                    else
                    {
                        Log::warning("Unrecognised default value:", current,
                                     "for option:", Syntax::commandString(type));
                    }

                    previous.clear();
                    continue;
                }

                _option.name = current;

                previous.clear();
                continue;
            }

            previous = current;
        }

        // All modifiers parsed. Final adjustments:
        if (type == Syntax::Command::Library)
        {
            std::filesystem::path filePath = _library.name;
            if (_library.type == Syntax::LibraryType::Static)
            {
                filePath.replace_extension(Syntax::Extension::LibraryStatic);
            }
            else if (_library.type == Syntax::LibraryType::Dynamic)
            {
                filePath.replace_extension(Syntax::Extension::LibraryDynamic);
            }
            // TODO: use different extension per platform!
            Log::verbose("Appending library file:", filePath.string());
            _object.name = filePath.string();
        }
    }
    else
    {
        if (type == Syntax::Command::Source)
        {
            std::filesystem::path filePath = _modifiers.back();
            filePath.replace_extension(Syntax::Extension::ObjectFile1);
            // TODO: use different extension per platform!
            Log::verbose("Appending object file:", filePath.string());
            _object.name = filePath.string();
        }
    }
}

bool Command::isReadyToExecute() const
{
    return _isReadyToExe;
}

void Command::setIsReadyToExecute(const bool ready)
{
    _isReadyToExe = ready;
}

std::string Command::whole() const
{
    std::string mods;

    for (const auto &current : _modifiers)
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
            extra = Space + Tools::inBrackets(Tools::listToString(_executable.objects));
            break;
        case Syntax::Command::Library:
            extra = Space + Tools::inBrackets(Tools::listToString(_library.objects));
            break;
        case Syntax::Command::Source:
            extra = Space + Tools::inBrackets(_object.name);
            break;
        case Syntax::Command::Include:
            if (_include.isLibrary)
                extra = Space + Tools::inBrackets(
                                    Syntax::commandString(Syntax::Command::Library));
            break;
        case Syntax::Command::Feature:
        case Syntax::Command::Option:
            extra =
                Space + Tools::inBrackets(std::string(Syntax::Modifier::Default) + Space +
                                          Tools::boolToString(_option.defaultValue));
            break;
        case Syntax::Command::Define:
        case Syntax::Command::Subproject:
        case Syntax::Command::Tool:
        case Syntax::Command::Qt:
        case Syntax::Command::Invalid:
        case Syntax::Command::Unknown:
            break;
        }
    }

    return Syntax::commandString(type) + mods + extra;
}

std::string Command::value() const
{
    if (_modifiers.empty())
    {
        return {};
    }

    return _modifiers.back();
}

std::string Command::path() const
{
    if (type == Syntax::Command::Include)
    {
        return _include.path;
    }
    else if (type == Syntax::Command::Source)
    {
        return _object.name;
    }
    else if (type == Syntax::Command::Executable)
    {
        return _executable.name;
    }
    else if (type == Syntax::Command::Library)
    {
        return _library.name;
    }

    switch (type)
    {
    case Syntax::Command::Include:
        return _include.path;
    case Syntax::Command::Source:
        return _object.name;
    case Syntax::Command::Executable:
        return _executable.name;
    case Syntax::Command::Library:
        return _library.name;
    case Syntax::Command::Define:
    case Syntax::Command::Feature:
    case Syntax::Command::Option:
    case Syntax::Command::Qt:
    case Syntax::Command::Subproject:
    case Syntax::Command::Tool:
    case Syntax::Command::Invalid:
    case Syntax::Command::Unknown:
        break;
    }

    Log::error("Path requested from command which does not support it:",
               Syntax::commandString(type), "available modifiers are:", _modifiers);

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
        _executable.objects.push_back(name);
    }
    else if (type == Syntax::Command::Library)
    {
        _library.objects.push_back(name);
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

const ExecutableComponent &Command::executable() const
{
    return _executable;
}

void Command::setExecutableName(const std::string &name)
{
    _executable.name = name;
}

const LibraryComponent &Command::library() const
{
    return _library;
}

const ObjectComponent &Command::object() const
{
    return _object;
}

const IncludeComponent &Command::include() const
{
    return _include;
}

const OptionComponent &Command::option() const
{
    return _option;
}

// General members
// const TargetId &Command::targetId() const
// {
//     return _targetId;
// }

// const CommandId &Command::parentId() const
// {
//     return _parentId;
// }

// const Syntax::Command &Command::type() const
// {
//     return _type;
// }

std::optional<Syntax::Command> Command::getCommand(const std::string &command) const
{
    try
    {
        const auto result = Syntax::commandValue(command);
        return result;
    }
    catch (const CommandStringException &e)
    {
        Log::verbose(e.what());
        return {};
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
    case Syntax::Command::Unknown:
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
