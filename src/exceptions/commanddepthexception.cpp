#include "commanddepthexception.h"
#include "project/command.h"

CommandDepthException::CommandDepthException(const Command &command)
    : std::exception(), message("Parent of command " + std::to_string(command.id()) +
                                " has not been found. Full: " + command.whole())
{
}

const char *CommandDepthException::what() const noexcept
{
    return message.c_str();
}
