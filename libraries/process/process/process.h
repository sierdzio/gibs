#pragma once

#include <string>
#include <vector>

struct Exit
{
    enum class Status
    {
        Success,
        FailedToExecute,
        FailedDuringExecution,
        UndefinedFailure
    };

    int rawCode = 0;
    Status status = Status::Success;
};

using Arguments = std::vector<std::string>;

class Process
{
  public:
    virtual ~Process() = default;

    void setExecutable(const std::string &filePath);
    std::string executable() const;

    void setArguments(const Arguments &args);
    Arguments arguments() const;

    Exit execute();

  protected:
    virtual Exit performAction() = 0;
    std::string argsToString(const Arguments &args) const;

  private:
    std::string _executablePath;
    Arguments _arguments;
};