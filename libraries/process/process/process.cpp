#include "process.h"

#include <functional>
#include <iostream>
#include <logger/log.h>

#if defined(__unix__) || defined(__APPLE__)
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace Tools
{
class ScopeGuard
{
  public:
    explicit ScopeGuard(const std::function<void()> &function) : _function(function)
    {
    }

    ~ScopeGuard()
    {
        if (_function)
        {
            _function();
        }
    }

  private:
    std::function<void()> _function;
};
} // namespace Tools

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
#if defined(__unix__) || defined(__APPLE__)
    std::cout.flush();

    const auto executablePath = executable();
    const auto argumentsCopy = arguments();

    if (executablePath.empty())
    {
        Log::error(logIdentifier(), "No executable specified for process");
        finish(1, Exit::Status::FailedToExecute);
        return;
    }

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

    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    const auto pipesGuard = Tools::ScopeGuard(
        [&]()
        {
            if (stdoutPipe[0] >= 0)
            {
                close(stdoutPipe[0]);
            }
            if (stdoutPipe[1] >= 0)
            {
                close(stdoutPipe[1]);
            }
            if (stderrPipe[0] >= 0)
            {
                close(stderrPipe[0]);
            }
            if (stderrPipe[1] >= 0)
            {
                close(stderrPipe[1]);
            }
        });

    if (pipe(stdoutPipe) != 0 || pipe(stderrPipe) != 0)
    {
        const int err = errno;
        Log::error(logIdentifier(), "Failed to create pipes:", std::strerror(err));
        finish(1, Exit::Status::FailedToExecute);
        return;
    }

    posix_spawn_file_actions_t fileActions;
    posix_spawn_file_actions_init(&fileActions);
    const auto fileActionsGuard =
        Tools::ScopeGuard([&]() { posix_spawn_file_actions_destroy(&fileActions); });
    posix_spawn_file_actions_adddup2(&fileActions, stdoutPipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&fileActions, stderrPipe[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&fileActions, stdoutPipe[0]);
    posix_spawn_file_actions_addclose(&fileActions, stderrPipe[0]);
    posix_spawn_file_actions_addclose(&fileActions, stdoutPipe[1]);
    posix_spawn_file_actions_addclose(&fileActions, stderrPipe[1]);

    pid_t childPid = 0;
    extern char **environ;
    const int spawnErr =
        posix_spawnp(&childPid, argv[0], &fileActions, nullptr, argv.data(), environ);
    if (spawnErr != 0)
    {
        Log::error(logIdentifier(), "Failed to spawn process:", executablePath,
                   "errno:", spawnErr, std::strerror(spawnErr));
        finish(1, Exit::Status::FailedToExecute);
        return;
    }

    close(stdoutPipe[1]);
    stdoutPipe[1] = -1;
    close(stderrPipe[1]);
    stderrPipe[1] = -1;

    std::string stdoutBuffer;
    std::string stderrBuffer;
    constexpr size_t bufferSize = 4096;
    std::vector<char> buffer(bufferSize);

    struct pollfd pollFds[2];
    pollFds[0].fd = stdoutPipe[0];
    pollFds[0].events = POLLIN | POLLHUP;
    pollFds[0].revents = 0;
    pollFds[1].fd = stderrPipe[0];
    pollFds[1].events = POLLIN | POLLHUP;
    pollFds[1].revents = 0;

    bool stdoutOpen = true;
    bool stderrOpen = true;
    while (stdoutOpen || stderrOpen)
    {
        const int ready = poll(pollFds, 2, -1);
        if (ready == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            Log::error(logIdentifier(), "poll failed:", std::strerror(errno));
            finish(1, Exit::Status::FailedDuringExecution);
            return;
        }

        for (int index = 0; index < 2; ++index)
        {
            if ((index == 0 && !stdoutOpen) || (index == 1 && !stderrOpen))
            {
                continue;
            }

            if ((pollFds[index].revents & POLLIN) == 0 &&
                (pollFds[index].revents & POLLHUP) == 0)
            {
                continue;
            }

            const int fd = pollFds[index].fd;
            const ssize_t count = read(fd, buffer.data(), buffer.size());
            if (count < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }

                Log::error(logIdentifier(),
                           "Failed to read process output:", std::strerror(errno));
                finish(1, Exit::Status::FailedDuringExecution);
                return;
            }

            if (count == 0)
            {
                if (index == 0)
                {
                    stdoutOpen = false;
                }
                else
                {
                    stderrOpen = false;
                }
                continue;
            }

            if (isLogProcessOutput())
            {
                if (index == 0)
                {
                    std::cout.write(buffer.data(), count);
                    std::cout.flush();
                }
                else
                {
                    std::cerr.write(buffer.data(), count);
                    std::cerr.flush();
                }
            }
            else
            {
                if (index == 0)
                {
                    stdoutBuffer.append(buffer.data(), static_cast<size_t>(count));
                }
                else
                {
                    stderrBuffer.append(buffer.data(), static_cast<size_t>(count));
                }
            }
        }
    }

    close(stdoutPipe[0]);
    stdoutPipe[0] = -1;
    close(stderrPipe[0]);
    stderrPipe[0] = -1;

    int status = 0;
    while (true)
    {
        const auto result = waitpid(childPid, &status, 0);
        if (result == childPid)
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
            if (!isLogProcessOutput())
            {
                if (!stdoutBuffer.empty())
                {
                    Log::error(logIdentifier(), "Process stdout:", stdoutBuffer);
                }
                if (!stderrBuffer.empty())
                {
                    Log::error(logIdentifier(), "Process stderr:", stderrBuffer);
                }
            }
            finish(1, Exit::Status::FailedDuringExecution);
            return;
        }
    }

    const bool processFailed = !WIFEXITED(status) || WEXITSTATUS(status) != 0;
    if (processFailed && !isLogProcessOutput())
    {
        if (!stdoutBuffer.empty())
        {
            Log::error(logIdentifier(), "Process stdout:", stdoutBuffer);
        }
        if (!stderrBuffer.empty())
        {
            Log::error(logIdentifier(), "Process stderr:", stderrBuffer);
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
    Log::error(logIdentifier(),
               "Process running is not implemented yet:", std::strerror(errno));
    finish(1, Exit::Status::FailedToExecute);
#endif
}