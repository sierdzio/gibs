#pragma once

#include <ostream>
#include <string>

#define TYPES                                                                            \
    X(Unknown, "unknown")                                                                \
    X(Executable, "executable")                                                          \
    X(Library, "library")

struct TargetId
{
#define X(key, name) key,
    enum class Type
    {
        TYPES
    };
#undef X

    static const std::string typeString(const Type type);
    static Type typeValue(const std::string &string);
    static size_t typesCount();

    TargetId();
    TargetId(std::string &&name, const Type type);

    bool operator<=>(const TargetId &other) const = default;
    bool isNull() const;

    const std::string &name() const;
    Type type() const;

  private:
    void setName(const std::string &name);

    std::string _name;
    Type _type = Type::Unknown;
    unsigned int _id = 0;
};

std::ostream &operator<<(std::ostream &stream, const TargetId &id);
