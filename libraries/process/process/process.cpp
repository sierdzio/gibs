#include "process.h"

#include <logger/log.h>

namespace
{
constexpr auto Space = " ";
} // namespace

void Process::setExecutable(const std::string &filePath)
{
    _executablePath = filePath;
}

std::string Process::executable() const
{
    return _executablePath;
}

void Process::setArguments(const Arguments &args)
{
    _arguments = args;
}

Arguments Process::arguments() const
{
    return _arguments;
}

Exit Process::execute()
{
    Exit result;

    Log::debug("Running process:");

    return result;
}

std::string Process::argsToString(const Arguments &args) const
{
    std::string result;

    for (const auto &arg : args)
    {
        if (not result.empty())
        {
            result.append(Space);
        }

        result.append(arg);
    }

    return result;
}
