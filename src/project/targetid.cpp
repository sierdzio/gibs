#include "targetid.h"
#include "exceptions/targetidtypeexception.h"

#include <logger/log.h>

#include <algorithm>
#include <array>

namespace
{
static unsigned int uniqueId = 1;

static unsigned int nextId()
{
    return uniqueId++;
}

#define X(key, name) name,
constexpr static std::array TypeStrings = {TYPES};
#undef X

void checkBounds(const TargetId::Type type)
{
    const auto raw = static_cast<size_t>(type);

    if (raw >= TypeStrings.size())
    {
        throw TargetIdTypeException(raw);
    }
}
} // namespace

const std::string TargetId::typeString(const Type type)
{
    checkBounds(type);
    return TypeStrings.at(static_cast<size_t>(type));
}

TargetId::Type TargetId::typeValue(const std::string &string)
{
    const auto it = std::find(TypeStrings.cbegin(), TypeStrings.cend(), string);

    if (it == TypeStrings.cend())
    {
        return Type::Unknown;
    }

    return static_cast<Type>(std::distance(TypeStrings.cbegin(), it));
}

size_t TargetId::typesCount()
{
    return TypeStrings.size();
}

TargetId::TargetId()
{
}

TargetId::TargetId(std::string &&name, const Type type) : _type(type)
{
    setName(name);
}

std::filesystem::path TargetId::rootDirectory() const
{
    return _rootDirectory;
}

void TargetId::setRootDirectory(const std::filesystem::path &path)
{
    _rootDirectory = path;
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

bool TargetId::operator==(const TargetId &other) const
{
    if (_id != 0 || other._id != 0)
    {
        return _id != 0 && other._id != 0 && _id == other._id;
    }

    return _name == other._name && _type == other._type &&
           _rootDirectory == other._rootDirectory;
}

std::ostream &operator<<(std::ostream &stream, const TargetId &id)
{
    stream << id.name() << ':' << TargetId::typeString(id.type()) << ':'
           << id.rootDirectory();

    return stream;
}
