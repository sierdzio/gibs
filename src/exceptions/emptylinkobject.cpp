#include "emptylinkobject.h"
#include "project/command.h"

EmptyLinkObject::EmptyLinkObject(const Command &command)
    : std::exception(),
      message("Attempted to add an empty link object to: " + command.whole())
{
}

const char *EmptyLinkObject::what() const noexcept
{
    return message.c_str();
}
