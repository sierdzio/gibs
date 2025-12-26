#include "systemprocess.h"

#include <logger/log.h>

#include <cstdlib>

void SystemProcess::performWork()
{
    Log::debug(logIdentifier(), " -> Executing process:", executable(),
               "with args:", argsToString(arguments()));

    std::cout.flush();
    const auto result = system(fullCommandLineCall().c_str());

    Log::debug(logIdentifier(), " -> Done! Process has finished:", executable(), "with exit code:", result);

    finish(result, result == 0 ? Exit::Status::Success : Exit::Status::UndefinedFailure);
}
