#include "processor.h"
#include "parsing/syntax.h"
#include "tools/log.h"

#include <process/stupidprocess.h>

#include <string>
#include <thread>
#include <vector>

struct Tool
{
    virtual std::string command() const = 0;
    virtual std::vector<std::string> arguments() const = 0;
};

struct Compiler : public Tool
{
    std::string command() const override
    {
        return "g++";
    }

    void setInputs(const std::string &filePath)
    {
        _filePath = filePath;
    }

    std::vector<std::string> arguments() const override
    {
        if (_filePath.empty())
        {
            Log::error("Cannot compile: no source file path!");
            return {};
        }

        std::vector<std::string> result;

        result.push_back("-c");
        result.push_back(_filePath);

        return result;
    }

  private:
    std::string _filePath;
};

struct Linker : public Tool
{
    std::string command() const override
    {
        return "g++";
    }

    void setInputs(const std::string &outputFilePath,
                   const std::vector<std::string> &objectFilePaths)
    {
        _outputFilePath = outputFilePath;
        _objectFilePaths = objectFilePaths;
    }

    std::vector<std::string> arguments() const override
    {
        std::vector<std::string> result;

        result.push_back("-S"); // TODO: static vs. dynamic
        result.push_back("-o");
        result.push_back(_outputFilePath);

        for (const auto &path : _objectFilePaths)
        {
            result.push_back(path);
        }

        return result;
    }

  private:
    std::string _outputFilePath;
    std::vector<std::string> _objectFilePaths;
};

Processor::Processor()
{
}

void Processor::schedule(const Command &command)
{
    Log::information("Scheduling command: ", command.whole());

    const auto typeString = Syntax::commandString(command.type);

    Process *process = nullptr;

    // TODO: actual threading or future, or async, or something
    //std::thread thread;
    //thread.detach();

    switch (command.type)
    {
    case Syntax::Command::Executable:
    case Syntax::Command::Library:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not implemented yet!");
        {
            Linker tool;

            tool.setInputs(command.executable().name, command.executable().objects);

            if (not isDryRun())
            {
                process = new StupidProcess;
                // TODO: get real data from command:
                process->setExecutable(tool.command());
                process->setArguments(tool.arguments());
            }
        }
        break;
    case Syntax::Command::Option:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not implemented yet!");
        break;
    case Syntax::Command::Qt:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not implemented yet!");
        break;
    case Syntax::Command::Source:
        Log::debug("Processing:", typeString, "command:", command.whole());
        Log::error("Not fully functional yet!");
        {
            Compiler tool;

            tool.setInputs(command.object().name);

            if (not isDryRun())
            {
                process = new StupidProcess;
                process->setExecutable(tool.command());
                process->setArguments(tool.arguments());
            }
        }

        break;
    case Syntax::Command::Tool:
        Log::debug("Processing:", typeString, "command:", command.whole());
        break;
    case Syntax::Command::Include:
    case Syntax::Command::Feature:
    case Syntax::Command::Subproject:
    case Syntax::Command::Define:
    case Syntax::Command::Invalid:
    case Syntax::Command::Unknown:
        Log::warning("This command type:", typeString, "does not need to be processed");
        return;
    }

    if (process and not isDryRun())
    {
        processes.insert({command.id(), process});
        process->execute();
    }
}

void Processor::setDryRun(const bool dryRun)
{
    _dryRun = dryRun;
}

bool Processor::isDryRun() const
{
    return _dryRun;
}