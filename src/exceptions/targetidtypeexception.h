#pragma once

#include <exception>
#include <string>

class TargetIdTypeException : public std::exception
{
  public:
    TargetIdTypeException(const size_t value);
    const char *what() const noexcept override;

  private:
    const std::string message;
};
