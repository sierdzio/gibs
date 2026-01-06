#include "compiler.h"
#include "project/command.h"

#include <logger/log.h>

#include <utility>

Compiler::Compiler(const Command &command)
{
    Compiler::setup(command);
}

std::string Compiler::command() const
{
    return "g++";
}

StringList Compiler::arguments() const
{
    return _arguments;
}

bool Compiler::setup(const Command &command)
{
    if (command.object().name.empty())
    {
        Log::error("Cannot compile: no source file path!");
        return false;
    }

    for (const auto &current : std::as_const(command.object().includePaths))
    {
        if (current.empty())
        {
            continue;
        }

        _arguments.emplace_back("-I");
        _arguments.emplace_back(current);
    }

    _arguments.emplace_back("-o");
    _arguments.emplace_back(command.object().name);

    _arguments.emplace_back(command.object().source);

    return true;
}
