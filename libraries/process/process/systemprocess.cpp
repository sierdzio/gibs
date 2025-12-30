#include "systemprocess.h"

#include <logger/log.h>

#include <cstdlib>

void SystemProcess::performWork()
{
    std::cout.flush();
    const auto result = system(fullCommandLineCall().c_str());

    finish(result, result == 0 ? Exit::Status::Success : Exit::Status::UndefinedFailure);
}
