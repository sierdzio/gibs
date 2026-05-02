#include "dryrunprocess.h"

DryRunProcess::DryRunProcess() : StupidProcess()
{
    setDuration(std::chrono::milliseconds(0));
}
