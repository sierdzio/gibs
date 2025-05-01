#pragma once

#include <exception>
#include <string>

class LogLevelException : public std::exception
{
  public:
    LogLevelException(const size_t value);
    const char *what() const noexcept override;

  private:
    const std::string message;
};
