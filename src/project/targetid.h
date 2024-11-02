#pragma once

#include <string>

struct TargetId
{
    TargetId();
    TargetId(std::string&& name);

    bool operator<=>(const TargetId& other) const = default;

    bool isNull() const;

    std::string name;
    uint id = 0;
};
