#pragma once

#include "tools/stringlist.h"

class Command;

struct Tool
{
    virtual bool setup(const Command &command) = 0;
    virtual std::string command() const = 0;
    virtual StringList arguments() const = 0;
};