#include "processexception.h"

#include <process/processinterface.h>

ProcessException::ProcessException(const CommandId &commandId,
                                   const ProcessInterface *process)
    : std::runtime_error("Process failed for command id: " + std::to_string(commandId) +
                         " call: " + process->fullCommandLineCall())
//TODO: also include whole process output if available
{
}