#pragma once

#include "tool.h"

class Linker : public Tool
{
  public:
    Linker(const Command &command);

    std::string command() const override;
    StringList arguments() const override;

  protected:
    bool setup(const Command &command) override;

  private:
    StringList _arguments;
    std::string _command;
};
