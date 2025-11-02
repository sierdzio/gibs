#pragma once

#include "tool.h"

struct Compiler : public Tool
{
    bool setup(const Command &command) override;
    std::string command() const override;
    StringList arguments() const override;

  private:
    StringList _arguments;
};
