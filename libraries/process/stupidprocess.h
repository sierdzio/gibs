#pragma once

#include "process.h"

class StupidProcess : public Process
{
  public:
    ~StupidProcess() = default;

    void setExecutable(const std::string &filePath) final;
    void setArguments(const std::vector<std::string> &args) final;
    void execute() final;

  private:
    std::string filePath;
    std::vector<std::string> args;
};