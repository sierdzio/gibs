#pragma once

#include <stdexcept>

class TargetIdTypeException : public std::runtime_error
{
  public:
    TargetIdTypeException(const size_t value);
};
