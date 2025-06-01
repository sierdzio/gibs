#pragma once

#include <exception>
#include <string>

class Command;

class EmptyLinkObject : public std::exception
{
  public:
    EmptyLinkObject(const Command &command);
    const char *what() const noexcept override;

  private:
    const std::string message;
};
