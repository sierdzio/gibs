#pragma once

#include "project/command.h"

#include <stdexcept>
#include <vector>

class CommandNotFound : public std::runtime_error
{
  public:
    CommandNotFound(const CommandId &id, const std::vector<Command> &commands);
};
