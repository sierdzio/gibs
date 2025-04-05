#include "targetid.h"

#include "tools/log.h"

namespace
{
static uint uniqueId = 0;

static uint nextId()
{
    return uniqueId++;
}
} // namespace

TargetId::TargetId()
{
}

TargetId::TargetId(std::string &&name, const Type type) : _type(type)
{
    setName(name);
}

bool TargetId::isNull() const
{
    return _name.empty();
}

const std::string &TargetId::name() const
{
    return _name;
}

TargetId::Type TargetId::type() const
{
    return _type;
}

void TargetId::setName(const std::string &name)
{
    if (isNull())
    {
        _name = name;
        _id = nextId();
    }
    else
    {
        // TODO: probably best throw something :-)
        Log::error("Cannot set target name twice. Attempting to change target name from:", _name,
                   "to:", name);
    }
}
