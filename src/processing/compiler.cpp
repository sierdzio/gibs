#include "compiler.h"
#include "project/command.h"

#include <logger/log.h>

#include <utility>

Compiler::Compiler(const Command &command)
{
    Compiler::setup(command);
}

const std::vector<CommandData> &Compiler::commands() const
{
    return _commands;
}

bool Compiler::setup(const Command &command)
{
    if (command.object().name.empty())
    {
        Log::error("Cannot compile: no source file path!");
        return false;
    }

    CommandData commandData;
    commandData.command = "g++";

    for (const auto &current : std::as_const(command.object().includePaths))
    {
        if (current.empty())
        {
            continue;
        }

        commandData.arguments.emplace_back("-I");
        commandData.arguments.emplace_back(current);
    }

    for (const auto &current : std::as_const(command.object().defines))
    {
        if (current.empty())
        {
            continue;
        }

        commandData.arguments.emplace_back("-D" + current);
    }

    commandData.arguments.emplace_back("-o");
    commandData.arguments.emplace_back(command.object().name);
    commandData.arguments.emplace_back(command.object().source);

    _commands.clear();
    _commands.emplace_back(std::move(commandData));

    return true;
}
