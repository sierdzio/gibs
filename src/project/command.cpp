#include "command.h"
#include "exceptions/commandexception.h"
#include "exceptions/emptylinkobject.h"
#include "parsing/syntax.h"
#include "tools/tools.h"

#include <logger/log.h>

#include <filesystem>
#include <typeindex>
#include <utility>

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

CommandId Command::id() const
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
        Log::warning(CommandNotValid,
                     "command requires a value and/ or modifiers but none have been "
                     "provided. All parsed elements:",
                     whole());
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

bool Command::canBeProcessed() const
{
    switch (type)
    {
    case Syntax::Command::Executable:
    case Syntax::Command::Library:
    case Syntax::Command::Option:
    case Syntax::Command::Qt:
    case Syntax::Command::Source:
    case Syntax::Command::Tool:
        return true;
    case Syntax::Command::Include:
    case Syntax::Command::Feature:
    case Syntax::Command::Subproject:
    case Syntax::Command::Define:
    case Syntax::Command::Invalid:
    case Syntax::Command::Unknown:
        return false;
    }

    return false;
}

void Command::finalize(const ArgumentsList &arguments, const Paths &paths)
{
    if (_modifiers.empty())
    {
        return;
    }

    if (supportsModifiers(type))
    {
        std::string previous;
        for (const auto &current : std::as_const(_modifiers))
        {
            if (type == Syntax::Command::Executable)
            {
                if (previous == Syntax::Modifier::Name)
                {
                    _executable.name = current;
                    previous.clear();
                    continue;
                }
                else if (_executable.name.empty() && current != Syntax::Modifier::Name)
                {
                    _executable.name = current;
                    continue;
                }
            }
            else if (type == Syntax::Command::Library)
            {
                if (previous == Syntax::Modifier::Library)
                {
                    previous.clear();
                }

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
                    const auto namePath = std::filesystem::path(current);
                    if (namePath.is_absolute())
                    {
                        _library.name =
                            std::filesystem::relative(namePath, paths.workingDirectory);
                    }
                    else
                    {
                        _library.name = current;
                    }
                    previous.clear();
                    continue;
                }
                else if (_library.name.empty() && current != Syntax::Modifier::Name &&
                         current != Syntax::Modifier::Type &&
                         current != Syntax::Modifier::Library)
                {
                    _library.name = current;
                    continue;
                }
                else if (not previous.empty())
                {
                    Log::warning("Unknown library modifier:", previous, current);
                }
            }
            else if (type == Syntax::Command::Include)
            {
                const auto currentPath =
                    std::filesystem::path(Tools::prepareIncludePath(current));
                const auto path = paths.absolutePath(currentPath);

                if (previous == Syntax::Modifier::Library)
                {
                    _include.isLibrary = true;
                    _include.path =
                        std::filesystem::relative(path, paths.workingDirectory);
                    if (not _modifiers.empty())
                    {
                        _modifiers.back() = _include.path;
                    }
                    previous.clear();
                    continue;
                }

                _include.path = std::filesystem::relative(path, paths.workingDirectory);
                if (not _modifiers.empty())
                {
                    _modifiers.back() = _include.path;
                }
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
                        Log::error("Unrecognised default value:", current,
                                   "for option:", Syntax::commandString(type));
                    }

                    if (const auto it = arguments.find(_option.name);
                        it != arguments.end())
                    {
                        const auto &value = it->second;

                        if (value.type() == std::type_index(typeid(bool)))
                        {
                            _option.isOn = std::any_cast<bool>(value);
                        }
                        else
                        {
                            Log::warning("Option:", _option.name,
                                         "has non-boolean value:", value.type().name(),
                                         "so default value will be used:",
                                         Tools::boolToString(_option.defaultValue));
                            _option.isOn = _option.defaultValue;
                        }
                    }
                    else
                    {
                        Log::information("Option:", _option.name, "using default value:",
                                         Tools::boolToString(_option.defaultValue));
                        _option.isOn = _option.defaultValue;
                    }

                    previous.clear();
                    continue;
                }
                else if (previous == Syntax::Modifier::Name)
                {
                    _option.name = current;
                    previous.clear();
                    continue;
                }
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
            const auto absoluteSource =
                filePath.is_absolute() ? filePath : paths.workingDirectory / filePath;
            _object.source =
                std::filesystem::relative(absoluteSource, paths.workingDirectory);
            if (not _modifiers.empty())
            {
                _modifiers.back() = _object.source;
            }

            filePath = absoluteSource;
            filePath.replace_extension(Syntax::Extension::ObjectFile1);
            // TODO: use different extension per platform!
            Log::verbose("Appending object file:", filePath.string());
            _object.name = filePath.filename().string();
        }
    }

    //Log::verbose("Finalized:", id(), parentId, "whole command:", whole());
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
            extra = Space +
                    Tools::inBrackets(Syntax::commandString(Syntax::Command::Library));
        break;
    case Syntax::Command::Feature:
    case Syntax::Command::Option:
        extra = Space + Tools::inBrackets(std::string(Syntax::Modifier::Default) + Space +
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

    std::string target = targetId.name();

    if (Log::isWithinLogLevel(Log::Type::Verbose))
    {
        target += Space + targetId.rootDirectory().string();
    }

    return Tools::inSquareBrackets(target) + Space + Syntax::commandString(type) + mods +
           extra;
}

std::string Command::value() const
{
    if (_modifiers.empty())
    {
        return {};
    }

    return _modifiers.back();
}

const std::string &Command::path() const
{
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

    throw CommandStringException("Path requested from command which does not support it");
}

bool Command::hasPath() const
{
    switch (type)
    {
    case Syntax::Command::Include:
    case Syntax::Command::Source:
    case Syntax::Command::Executable:
    case Syntax::Command::Library:
        return true;
    case Syntax::Command::Define:
    case Syntax::Command::Feature:
    case Syntax::Command::Option:
    case Syntax::Command::Qt:
    case Syntax::Command::Subproject:
    case Syntax::Command::Tool:
    case Syntax::Command::Invalid:
    case Syntax::Command::Unknown:
        return false;
    }

    return false;
}

bool Command::addLinkObject(const std::string &name)
{
    if (name.empty())
    {
        throw EmptyLinkObject(*this);
    }

    if (type == Syntax::Command::Executable)
    {
        if (name.ends_with(Syntax::Extension::ObjectFile1) or
            name.ends_with(Syntax::Extension::ObjectFile2))
        {
            Log::verbose("Appending object file:", name,
                         "to executable command:", whole());
            _executable.objects.emplace_back(name);
        }
        else
        {
            Log::verbose("Appending library file:", name,
                         "to executable command:", whole());
            _executable.libraries.emplace_back(name);
        }
    }
    else if (type == Syntax::Command::Library)
    {
        _library.objects.emplace_back(name);
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

    if (type != Syntax::Command::Executable)
    {
        return;
    }

    if (_modifiers.empty())
    {
        _modifiers.emplace_back(name);
        return;
    }

    const auto it =
        std::find(_modifiers.begin(), _modifiers.end(), Syntax::Modifier::Name);
    if (it != _modifiers.end() && std::next(it) != _modifiers.end())
    {
        *std::next(it) = name;
        return;
    }

    if (_modifiers.size() == 1)
    {
        _modifiers[0] = name;
        return;
    }

    _modifiers.back() = name;
}

const LibraryComponent &Command::library() const
{
    return _library;
}

const ObjectComponent &Command::object() const
{
    return _object;
}

ObjectComponent &Command::objectReference()
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
