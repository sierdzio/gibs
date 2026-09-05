#include "compiler.h"
#include "project/command.h"

#include <logger/log.h>

#include <utility>

Compiler::Compiler(const Command &command, const CompilerSet &compilerSet)
    : _compilerSet(compilerSet)
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
    commandData.command = _compilerSet.compilerCommand;

    // TODO: add C++ version standard argument based on implementation in compiler
    // set. Also, make it modifyable by gibs command line arguments
    commandData.arguments.emplace_back("-std=c++23");

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
    commandData.arguments.emplace_back("-c");
    commandData.arguments.emplace_back(command.object().sourcePath.empty()
                                           ? command.object().source
                                           : command.object().sourcePath.string());

    _commands.clear();
    _commands.emplace_back(std::move(commandData));

    return true;
}
