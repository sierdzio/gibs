#pragma once

#include <string>
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

    bool execute();

    Exit result() const;

  protected:
    virtual void start() = 0;
    std::string argsToString(const Arguments &args) const;

    Exit _result;

  private:
    std::string _executablePath;
    Arguments _arguments;
};