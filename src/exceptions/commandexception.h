#pragma once

#include <stdexcept>
#include <string>

class CommandException : public std::runtime_error
{
  public:
    CommandException(const size_t value);
};

class CommandStringException : public std::runtime_error
{
  public:
    CommandStringException(const std::string &value);
};
