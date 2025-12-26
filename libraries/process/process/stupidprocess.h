#pragma once

#include "process.h"

#include <chrono>

using namespace std::chrono_literals;

/*!
  A simple process "simulator" - prints a message every second and exits after 5 seconds.
*/
class StupidProcess : public Process
{
  public:
    virtual ~StupidProcess() = default;

    void setDuration(const std::chrono::milliseconds duration);
    std::chrono::milliseconds duration() const;

  protected:
    void performWork() override;

  private:
    std::chrono::milliseconds _duration { 1s };
};
