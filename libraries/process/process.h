#pragma once

#include <string>
#include <vector>

class Process
{
  public:
    virtual ~Process() = default;

    virtual void setExecutable(const std::string &filePath) = 0;
    virtual void setArguments(const std::vector<std::string> &args) = 0;
    virtual void execute() = 0;
};