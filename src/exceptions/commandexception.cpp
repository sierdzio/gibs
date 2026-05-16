#include "commandexception.h"
#include "parsing/syntax.h"

#include <string>

CommandException::CommandException(const size_t value)
    : std::exception(),
      message("Invalid Command type: " + std::to_string(value) + " is out of range 0-" +
              std::to_string(Syntax::commandCount()))
{
}

const char *CommandException::what() const noexcept
{
    return message.c_str();
}

CommandStringException::CommandStringException(const std::string &value)
    : std::exception(), message("Command exception: " + value)
{
}

const char *CommandStringException::what() const noexcept
{
    return message.c_str();
}
