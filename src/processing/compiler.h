#pragma once

#include "compilerset.h"
#include "tool.h"

struct Compiler : public Tool
{
  public:
    Compiler(const Command &command,
             const CompilerSet &compilerSet = CompilerSet::defaultForPlatform());

    const std::vector<CommandData> &commands() const override;

  protected:
    bool setup(const Command &command) override;

  private:
    std::vector<CommandData> _commands;
    CompilerSet _compilerSet = CompilerSet::defaultForPlatform();
};
