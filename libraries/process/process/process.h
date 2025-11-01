#pragma once

#include <string>
#include <thread>
#include <vector>

struct Exit
{
    enum class Status
    {
        NotExecuted,
        InProgress,
        Success,
        FailedToExecute,
        FailedDuringExecution,
        UndefinedFailure,
    };

    int rawCode = -1;
    Status status = Status::NotExecuted;
};

using Arguments = std::vector<std::string>;

class Process
{
  public:
    virtual ~Process();

    void setExecutable(const std::string &filePath);
    std::string executable() const;

    void setArguments(const Arguments &args);
    Arguments arguments() const;

    bool start();
    bool isFinished() const;

    Exit result() const;

  protected:
    virtual void performWork() = 0;
    void finish(const int code, const Exit::Status status);
    std::string argsToString(const Arguments &args) const;

    Exit _result;

  private:
    std::string _executablePath;
    Arguments _arguments;
    std::thread _thread;
};