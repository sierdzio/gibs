#pragma once

#include "tool.h"

struct Compiler : public Tool
{
  public:
    Compiler(const Command &command);

    std::string command() const override;
    StringList arguments() const override;

  protected:
    bool setup(const Command &command) override;

  private:
    StringList _arguments;
};
