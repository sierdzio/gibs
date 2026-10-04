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
    /*!
     * Returns a CompilerSet based on provided \a name which can be one of the built-in
     * compiler sets: or a path to an ini file with custom configuration. If the name is
     * not recognized, the default compiler set for the platform is returned.
     */
    static CompilerSet fromName(const std::string &name);
    static bool isKnownName(const std::string &name);
};
