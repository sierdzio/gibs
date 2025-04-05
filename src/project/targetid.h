#pragma once

#include <string>

struct TargetId
{
    enum class Type
    {
        Unknown,
        Executable,
        Library,
    };

    TargetId();
    TargetId(std::string&& name, const Type type);

    bool operator<=>(const TargetId& other) const = default;
    bool isNull() const;

    const std::string& name() const;
    Type type() const;

private:
    void setName(const std::string& name);

    std::string _name;
    Type _type = Type::Unknown;
    uint _id = 0;
};
