#include "linker.h"
#include "project/command.h"

#include <logger/log.h>

Linker::Linker(const Command &command)
{
    Linker::setup(command);
}

const std::vector<CommandData> &Linker::commands() const
{
    return _commands;
}

bool Linker::setup(const Command &command)
{
    const bool isExe = command.type == Syntax::Command::Executable;
    const bool isStatic = command.library().type == Syntax::LibraryType::Static;

    _commands.clear();

    const auto &objects =
        isExe ? command.executable().objects : command.library().objects;

    if (isExe)
    {
        CommandData commandData;
        commandData.command = "g++";
        for (const auto &current : objects)
        {
            commandData.arguments.emplace_back(current);
        }
        commandData.arguments.emplace_back("-o");
        commandData.arguments.emplace_back(command.executable().name);
        _commands.emplace_back(std::move(commandData));

        return true;
    }

    if (isStatic)
    {
        const std::string archiveName = command.object().name;

        CommandData arCommand;
        arCommand.command = "ar";
        arCommand.arguments.emplace_back("qc");
        arCommand.arguments.emplace_back(archiveName);
        for (const auto &current : objects)
        {
            arCommand.arguments.emplace_back(current);
        }

        CommandData ranlibCommand;
        ranlibCommand.command = "ranlib";
        ranlibCommand.arguments.emplace_back(archiveName);

        _commands.emplace_back(std::move(arCommand));
        _commands.emplace_back(std::move(ranlibCommand));

        return true;
    }

    // Dynamic library path
    CommandData commandData;
    commandData.command = "g++";
    for (const auto &current : objects)
    {
        commandData.arguments.emplace_back(current);
    }
    commandData.arguments.emplace_back("-shared");
    commandData.arguments.emplace_back("-o");
    commandData.arguments.emplace_back(command.object().name);

    _commands.emplace_back(std::move(commandData));

    return true;
}
