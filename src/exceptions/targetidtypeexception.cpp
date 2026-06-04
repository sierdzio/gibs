#include "targetidtypeexception.h"
#include "project/targetid.h"

#include <string>

TargetIdTypeException::TargetIdTypeException(const size_t value)
    : std::runtime_error("Invalid TargetId type: " + std::to_string(value) +
                         " is out of range 0-" + std::to_string(TargetId::typesCount()))
{
}
