#pragma once

#include <stdexcept>

class Command;

class CommandDepthException : public std::runtime_error
{
  public:
    CommandDepthException(const Command &command);
};
