#pragma once

#include <string>

struct CompilerSet
{
    std::string name;
    std::string compilerCommand;
    std::string archiverCommand;
    std::string ranlibCommand;

    static CompilerSet defaultForPlatform();
    static CompilerSet fromName(const std::string &name);
    static bool isKnownName(const std::string &name);
};
