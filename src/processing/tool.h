#pragma once

#include "tools/stringlist.h"

class Command;

class Tool
{
  public:
    virtual std::string command() const = 0;
    virtual StringList arguments() const = 0;

  protected:
    virtual bool setup(const Command &command) = 0;
};
