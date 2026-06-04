#pragma once

#include "project/command.h"

#include <stdexcept>

class ProcessInterface;

class ProcessException : public std::runtime_error
{
  public:
    ProcessException(const CommandId &commandId, const ProcessInterface *process);
};
