#pragma once

#include "processinterface.h"

class Process : public ProcessInterface
{
  public:
    virtual ~Process() = default;

  protected:
    void performWork() override;

  private:
    void performWorkNatively();
};