#include "compilerset.h"

#include "tools/tools.h"

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

    return defaultForPlatform();
}

bool CompilerSet::isKnownName(const std::string &name)
{
    const auto normalized = Tools::toLower(name);
    return normalized == "gcc" or normalized == "clang" or normalized == "apple-clang";
}
