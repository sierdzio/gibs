#pragma once

#include <string>

struct CompilerSet
{
    enum class CommandExecution
    {
        Sequential,
        Parallel,
    };

    std::string name;
    std::string compilerCommand;
    std::string archiverCommand;
    std::string ranlibCommand;
    CommandExecution commandExecution = CommandExecution::Sequential;

    static CompilerSet defaultForPlatform();
    static CompilerSet fromName(const std::string &name);
    static bool isKnownName(const std::string &name);
};
