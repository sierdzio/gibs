#pragma once

#include "stupidprocess.h"

/*!
  Dry run version of Process - does not execute any real processes.

  This is equivalent to running StupidProcess but with the duration set to 0.
*/
class DryRunProcess : public StupidProcess
{
  public:
    DryRunProcess();
    virtual ~DryRunProcess() = default;
};
