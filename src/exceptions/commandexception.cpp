#include "commandexception.h"
#include "parsing/syntax.h"

#include <string>

CommandException::CommandException(const size_t value)
    : std::runtime_error("Invalid Command type: " + std::to_string(value) +
                         " is out of range 0-" + std::to_string(Syntax::commandCount()))
{
}

CommandStringException::CommandStringException(const std::string &value)
    : std::runtime_error("Command exception: " + value)
{
}
