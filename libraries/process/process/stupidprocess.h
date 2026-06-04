#pragma once

#include "processinterface.h"

#include <chrono>

using namespace std::chrono_literals;

/*!
  A simple process "simulator" - prints a message every second and exits after
  waiting for 5 times duration().

  Objects of this class will never actually run any real processes.
*/
class StupidProcess : public ProcessInterface
{
  public:
    virtual ~StupidProcess() = default;

    void setDuration(const std::chrono::milliseconds duration);
    std::chrono::milliseconds duration() const;

  protected:
    void performWork() override;

  private:
    std::chrono::milliseconds _duration{500ms};
};
