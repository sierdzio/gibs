#pragma once

#include "tool.h"

class Linker : public Tool
{
  public:
    Linker(const Command &command);

    const std::vector<CommandData> &commands() const override;

  protected:
    bool setup(const Command &command) override;

  private:
    std::vector<CommandData> _commands;
};
