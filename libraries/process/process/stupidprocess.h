#pragma once

#include "process.h"

class StupidProcess : public Process
{
  public:
    virtual ~StupidProcess() = default;

  protected:
    void start() override;
};