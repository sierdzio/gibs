#include "targetid.h"

namespace {
    static uint uniqueId = 0;

    static uint nextId()
    {
        return uniqueId++;
    }
}

TargetId::TargetId() : id(nextId())
{}

TargetId::TargetId(std::string&& name) : name(name), id(nextId())
{}

bool TargetId::isNull() const
{
    return name.empty();
}
