#include "linker.h"
#include "project/command.h"

#include <logger/log.h>

bool Linker::setup(const Command &command)
{
    const bool isExe = command.type == Syntax::Command::Executable;
    const auto &objects =
        isExe ? command.executable().objects : command.library().objects;

    for (const auto &current : objects)
    {
        _arguments.push_back(current);
    }

    if (not isExe)
    {
        _arguments.push_back(command.library().type == Syntax::LibraryType::Dynamic
                                 ? "-shared"
                                 : "-static");
    }

    _arguments.push_back("-o");
    _arguments.push_back(isExe ? command.executable().name : command.library().name);

    return true;
}

std::string Linker::command() const
{
    return "g++";
}

StringList Linker::arguments() const
{
    return _arguments;
}