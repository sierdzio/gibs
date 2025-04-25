#pragma once

#include <exception>
#include <string>

struct Command;

class CommandDepthException : public std::exception
{
  public:
    CommandDepthException(const Command &command);
    const char *what() const noexcept override;

  private:
    const std::string message;
};
