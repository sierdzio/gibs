#pragma once

#include "tool.h"

struct Compiler : public Tool
{
  public:
    Compiler(const Command &command);

    const std::vector<CommandData> &commands() const override;

  protected:
    bool setup(const Command &command) override;

  private:
    std::vector<CommandData> _commands;
};
