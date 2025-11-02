#pragma once

#include "process.h"

/*!
  A simple process "simulator" - prints a message every second and exits after 5 seconds.
*/
class StupidProcess : public Process
{
  public:
    virtual ~StupidProcess() = default;

  protected:
    void performWork() override;
};