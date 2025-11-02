#pragma once

#include "tool.h"

struct Linker : public Tool
{
    bool setup(const Command &command) override;
    std::string command() const override;
    StringList arguments() const override;

  private:
    StringList _arguments;
};