#include "targetid.h"
#include "tools/log.h"

#include <algorithm>

namespace
{
static uint uniqueId = 0;

static uint nextId()
{
    return uniqueId++;
}
} // namespace

const std::string TargetId::typeString(const Type type)
{
    return typeStrings.at(static_cast<size_t>(type));
}

TargetId::Type TargetId::typeValue(const std::string &string)
{
    const auto it = std::find(typeStrings.cbegin(), typeStrings.cend(), string);

    if (it == typeStrings.cend())
    {
        return Type::Unknown;
    }

    return static_cast<Type>(std::distance(typeStrings.cbegin(), it));
}

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
        Log::error("Cannot set target name twice. Attempting to change target name from:",
                   _name, "to:", name);
    }
}

std::ostream &operator<<(std::ostream &stream, const TargetId &id)
{
    stream << id.name() << ":" << TargetId::typeString(id.type());

    return stream;
}
