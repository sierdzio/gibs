#pragma once

#include <exception>
#include <string>

class CommandException : public std::exception
{
  public:
    CommandException(const size_t value);
    const char *what() const noexcept override;

  private:
    const std::string message;
};

class CommandStringException : public std::exception
{
  public:
    CommandStringException(const std::string &value);
    const char *what() const noexcept override;

  private:
    const std::string message;
};
