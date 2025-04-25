#include "commandnotfound.h"
#include "tools/log.h"

CommandNotFound::CommandNotFound(const CommandId &id,
                                 const std::vector<Command> &commands)
    : std::exception()
{
    message = "Command " + std::to_string(id) + " has not been found. ";

    if (Log::isWithinLogLevel(Log::Type::Verbose))
    {
        message += "Present commands are:\n";

        for (const auto &current : commands)
        {
            message += " |- " + current.whole() + '\n';
        }
    }
    else
    {
        message += "To see all parsed commands, launch gibs with --verbose flag.";
    }
}

const char *CommandNotFound::what() const noexcept
{
    return message.c_str();
}
