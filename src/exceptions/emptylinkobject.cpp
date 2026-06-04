#include "emptylinkobject.h"
#include "project/command.h"

EmptyLinkObject::EmptyLinkObject(const Command &command)
    : std::runtime_error("Attempted to add an empty link object to: " + command.whole())
{
}
