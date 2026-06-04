#pragma once

#include <stdexcept>

class Command;

class EmptyLinkObject : public std::runtime_error
{
  public:
    EmptyLinkObject(const Command &command);
};
