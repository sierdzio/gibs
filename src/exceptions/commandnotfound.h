#pragma once

#include "project/command.h"

#include <exception>
#include <string>
#include <vector>

class CommandNotFound : public std::exception
{
  public:
    CommandNotFound(const CommandId &id, const std::vector<Command> &commands);
    const char *what() const noexcept override;

  private:
    std::string message;
};
