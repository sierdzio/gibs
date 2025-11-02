#include "compiler.h"
#include "project/command.h"

#include <logger/log.h>

bool Compiler::setup(const Command &command)
{
    if (command.object().name.empty())
    {
        Log::error("Cannot compile: no source file path!");
        return false;
    }

    for (const auto &current : command.object().includePaths)
    {
        if (current.empty())
        {
            continue;
        }

        _arguments.push_back("-I");
        _arguments.push_back(current);
    }

    _arguments.push_back(command.object().name);

    return true;
}

std::string Compiler::command() const
{
    return "g++";
}

StringList Compiler::arguments() const
{
    return _arguments;
}