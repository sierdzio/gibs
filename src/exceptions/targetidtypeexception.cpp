#include "targetidtypeexception.h"
#include "project/targetid.h"

#include <string>

TargetIdTypeException::TargetIdTypeException(const size_t value)
    : std::exception(),
      message("Invalid TargetId type: " + std::to_string(value) + " is out of range 0-" +
              std::to_string(TargetId::typesCount()))
{
}

const char *TargetIdTypeException::what() const noexcept
{
    return message.c_str();
}
