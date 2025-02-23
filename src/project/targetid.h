#pragma once

#include <string>

struct TargetId
{
    TargetId();
    TargetId(std::string&& name);

    bool operator<=>(const TargetId& other) const = default;
    bool isNull() const;

    const std::string& name() const;

private:
    void setName(const std::string& name);

    std::string _name;
    uint _id = 0;
};
