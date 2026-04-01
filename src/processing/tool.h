#pragma once

#include "tools/stringlist.h"

#include <vector>

class Command;

struct CommandData
{
    std::string command;
    StringList arguments;
};

class Tool
{
  public:
    virtual const std::vector<CommandData> &commands() const = 0;

    std::string command() const
    {
        const auto &cmds = commands();
        return cmds.empty() ? std::string() : cmds.front().command;
    }

    StringList arguments() const
    {
        const auto &cmds = commands();
        return cmds.empty() ? StringList{} : cmds.front().arguments;
    }

  protected:
    virtual bool setup(const Command &command) = 0;
};
