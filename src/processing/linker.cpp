#include "linker.h"
#include "project/command.h"

#include <logger/log.h>

Linker::Linker(const Command &command)
{
    Linker::setup(command);
}

std::string Linker::command() const
{
    return _command;
}

StringList Linker::arguments() const
{
    return _arguments;
}

bool Linker::setup(const Command &command)
{
    const bool isExe = command.type == Syntax::Command::Executable;
    const bool isStatic = command.library().type == Syntax::LibraryType::Static;

    // TODO: based on library type, populate different commands
    // /usr/bin/ar qc libmultiple-files-and-libs-lib.a "CMakeFiles/multiple-files-and-libs-lib.dir/exported.cpp.o" "CMakeFiles/multiple-files-and-libs-lib.dir/libraryclass.cpp.o"
    // /usr/bin/ranlib libmultiple-files-and-libs-lib.a

    if (isStatic and not isExe)
    {
        _command = "ar";
    }
    else
    {
        _command = "g++";
    }

    const auto &objects =
        isExe ? command.executable().objects : command.library().objects;

    for (const auto &current : objects)
    {
        _arguments.emplace_back(current);
    }

    if (not isExe)
    {
        _arguments.emplace_back(command.library().type == Syntax::LibraryType::Dynamic
                                    ? "-shared"
                                    : "-static");
    }

    _arguments.emplace_back("-o");
    _arguments.emplace_back(isExe ? command.executable().name : command.library().name);

    return true;
}
