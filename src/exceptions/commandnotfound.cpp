#include "commandnotfound.h"

#include "tools/log.h"
#include <string>

CommandNotFound::CommandNotFound(const CommandId& id, const std::vector<Command>& commands)
{
    message = "Command " + std::to_string(id) + " has not been found.";

    if (Log::isWithinLogLevel(Log::Type::Verbose))
    {
        message += " Present commands are:\n";

        for (const auto& current : commands)
        {
            message += " |- " + current.whole() + '\n';
        }
    }
}

const char* CommandNotFound::what() const noexcept
{
    return message.c_str();
}
