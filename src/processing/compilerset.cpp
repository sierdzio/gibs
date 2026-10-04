#include "compilerset.h"
#include "tools/tools.h"

#include <logger/log.h>

//i include libraries/streamer
#include <streamer/ini.h>

namespace
{
constexpr std::string_view SectionName = "compiler-set";
constexpr std::string_view Name = "name";
constexpr std::string_view Compiler = "compiler";
constexpr std::string_view Archiver = "archiver";
constexpr std::string_view Ranlib = "ranlib";
constexpr std::string_view Execution = "execution";
} //namespace

CompilerSet CompilerSet::defaultForPlatform()
{
#ifdef __APPLE__
    return {"apple-clang", "clang++", "ar", "ranlib",
            CompilerSet::CommandExecution::Sequential};
#else
    return {"gcc", "g++", "ar", "ranlib", CompilerSet::CommandExecution::Sequential};
#endif
}

CompilerSet CompilerSet::fromName(const std::string &name)
{
    const auto normalized = Tools::toLower(name);

    if (normalized == "gcc")
    {
        return {"gcc", "g++", "ar", "ranlib", CompilerSet::CommandExecution::Sequential};
    }

    if (normalized == "clang")
    {
        return {"clang", "clang++", "ar", "ranlib",
                CompilerSet::CommandExecution::Sequential};
    }

    if (normalized == "apple-clang")
    {
        return {"apple-clang", "clang++", "ar", "ranlib",
                CompilerSet::CommandExecution::Sequential};
    }

    if (std::filesystem::exists(name))
    {
        Ini<Object> ini;
        if (not ini.read(name))
        {
            Log::error("Failed to read compiler set ini file:", name);
            // TODO: throw instead
            return {};
        }

        for (const auto &[section, entry] : ini)
        {
            Log::information("INI section:", section);

            if (section != SectionName)
            {
                continue;
            }

            const auto &object = entry.asObject();

            CompilerSet set;
            set.name = object.at(Name.data()).toString();
            set.compilerCommand = object.at(Compiler.data()).toString();
            set.archiverCommand = object.at(Archiver.data()).toString();
            set.ranlibCommand = object.at(Ranlib.data()).toString();
            const auto &execution = object.at(Execution.data()).toString();
            set.commandExecution = execution == "Parallel"
                                       ? CompilerSet::CommandExecution::Parallel
                                       : CompilerSet::CommandExecution::Sequential;

            return set;
        }
    }

    return defaultForPlatform();
}

bool CompilerSet::isKnownName(const std::string &name)
{
    const auto normalized = Tools::toLower(name);
    return normalized == "gcc" or normalized == "clang" or normalized == "apple-clang";
}
