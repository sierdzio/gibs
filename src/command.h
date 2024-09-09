#pragma once

#include <string>
#include <vector>

struct Command
{
    bool isValid() const;
    std::vector<std::string> whole;
};
