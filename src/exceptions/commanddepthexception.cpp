#include "commanddepthexception.h"
#include "project/command.h"

CommandDepthException::CommandDepthException(const Command &command)
    : std::runtime_error("Parent of command " + std::to_string(command.id()) +
                         " has not been found. Full: " + command.whole())
{
}
