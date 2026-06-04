#pragma once

#include <mutex>
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

/*!
 * Abstract base class for running processes. Contains basic API for setting up
 * the process, running it and getting results.
 */
class Process
{
  public:
    Process();
    virtual ~Process();

    void setExecutable(const std::string &filePath);
    const std::string &executable() const;

    void setArguments(const Arguments &args);
    const Arguments &arguments() const;

    void setMetaInformation(const std::string &information);
    const std::string &metaInformation() const;

    void setLogProcessOutput(const bool enabled);
    bool isLogProcessOutput() const;

    bool start();
    bool isFinished() const;
    Exit result() const;

    std::string fullCommandLineCall() const;

  protected:
    virtual void performWork() = 0;
    void finish(const int code, const Exit::Status status);
    std::string argsToString(const Arguments &args) const;
    bool hasMeta() const;
    std::string logMeta() const;
    unsigned int identifier() const;
    std::string logIdentifier() const;

    Exit _result;
    mutable std::mutex _mutex;

  private:
    static unsigned int _globalIdentifier;

    std::string _executablePath;
    Arguments _arguments;
    std::string _metaInformation;
    std::thread _thread;
    unsigned int _identifier = 0;
    bool _logProcessOutput = false;
};
