#pragma once

#include "processinterface.h"

/*!
 * Executes a process using system() call.
 *
 * \warning It is advised against using this process type in production code.
 * Shares all limitations of system() - not much is known about execution other
 * than exit status, there are security concerns with it etc.
 */
class SystemProcess : public ProcessInterface
{
  public:
    virtual ~SystemProcess() = default;

  protected:
    void performWork() override;
};
