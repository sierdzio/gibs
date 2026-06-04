#include "process.h"

#include <iostream>
#include <logger/log.h>

#if defined(__linux__)
#include <cerrno>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#endif

void Process::performWork()
{
    const auto fullInvocation = fullCommandLineCall();

    if (isLogProcessOutput())
    {
        Log::verbose(logIdentifier(), " -> Starting process:", fullInvocation);
    }

    performWorkNatively();
}

void Process::performWorkNatively()
{
#if defined(__linux__)
    std::cout.flush();

    const auto pid = fork();
    if (pid < 0)
    {
        Log::error(logIdentifier(), "Failed to fork process:", std::strerror(errno));
        finish(1, Exit::Status::FailedToExecute);
        return;
    }

    const auto executablePath = executable();
    const auto argumentsCopy = arguments();

    std::vector<std::string> argvStrings;
    argvStrings.reserve(1 + argumentsCopy.size());
    argvStrings.push_back(executablePath);
    for (const auto &arg : argumentsCopy)
    {
        argvStrings.push_back(arg);
    }

    std::vector<char *> argv;
    argv.reserve(argvStrings.size() + 1);
    for (auto &arg : argvStrings)
    {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    if (pid == 0)
    {
        execvp(argv[0], argv.data());
        const int err = errno;
        Log::error(logIdentifier(), "Failed to execute process:", executablePath,
                   "errno:", err, std::strerror(err));
        _exit(127);
    }

    int status = 0;
    while (true)
    {
        const auto result = waitpid(pid, &status, 0);
        if (result == pid)
        {
            break;
        }

        if (result == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            Log::error(logIdentifier(), "waitpid failed:", std::strerror(errno));
            finish(1, Exit::Status::FailedDuringExecution);
            return;
        }
    }

    if (WIFEXITED(status))
    {
        const int exitCode = WEXITSTATUS(status);
        finish(exitCode, exitCode == 0 ? Exit::Status::Success
                                       : Exit::Status::FailedDuringExecution);
        return;
    }

    if (WIFSIGNALED(status))
    {
        const int signalNumber = WTERMSIG(status);
        finish(-signalNumber, Exit::Status::FailedDuringExecution);
        return;
    }

    finish(status, Exit::Status::UndefinedFailure);
#elif defined(_WIN32)
#elif defined(__APPLE__)
#endif
}