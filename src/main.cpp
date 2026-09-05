//i executable name gibs version 0.2.0
//i include ../libraries/logger
//i include ../libraries/process

#include <chrono>
#include <iostream>
#include <stdexcept>

#include <logger/log.h>

#include "exceptions/commanddepthexception.h"
#include "exceptions/commandexception.h"
#include "exceptions/commandnotfound.h"
#include "exceptions/emptylinkobject.h"
#include "exceptions/processexception.h"
#include "exceptions/targetidtypeexception.h"
#include "parsing/parser.h"
#include "processing/compilerset.h"
#include "processing/processor.h"
#include "project/project.h"
#include "tools/commandline.h"

int main(int argc, char *argv[])
{
    const auto begin = std::chrono::steady_clock::now();
    const CommandLine cmd(CommandLine::toStringList(argc, argv));

    try
    {
        if (not cmd.isValid())
        {
            std::cout << "Error when parsing command line!" << std::endl;
            std::cout << cmd.helpText() << std::endl;
            return -1;
        }

        if (cmd.hasHelp())
        {
            std::cout << cmd.helpText() << std::endl;
            return 0;
        }
    }
    catch (const std::overflow_error &e)
    {
        std::cout << "Failed to generate help text:" << e.what() << std::endl;
        return -2;
    }

    if (cmd.hasVersion())
    {
        std::cout << cmd.versionText() << std::endl;
        return 0;
    }

    Log::setUseColorfulLogs(cmd.colorfulLogs());
    Log::information("Setting log level to:", Log::typeString(cmd.logLevel()));
    Log::setLogLevel(cmd.logLevel());
    if (cmd.isLogFilePathSet())
    {
        try
        {
            Log::setLogFile(cmd.logFilePath());
        }
        catch (const std::runtime_error &e)
        {
            Log::error("Failed to set log file path:", e.what());
            return -3;
        }
    }
    Log::debug(cmd.parsedFlagsText());

    auto processor = std::make_shared<Processor>();
    processor->setDryRun(cmd.isDryRun());
    processor->setLogProcessOutput(cmd.isLogProcessOutput());
    processor->setCompilerSet(CompilerSet::fromName(cmd.compilerSet()));

    auto project = std::make_shared<Project>(processor);

    Parser parser(cmd.input(), cmd.isQuickMode(), cmd.otherArguments(), project);

    if (parser.status() != AppError::NoError)
    {
        return static_cast<int>(parser.status());
    }

    int result = 0;

    try
    {
        parser.parse();

        if (parser.status() != AppError::NoError)
        {
            result = static_cast<int>(parser.status());
        }
    }
    catch (const CommandNotFound &e)
    {
        Log::error(e.what());
        result = -4;
    }
    catch (const CommandDepthException &e)
    {
        // Warning because it is a missing functionality but not crucial
        Log::warning(e.what());
        result = -5;
    }
    catch (const CommandException &e)
    {
        Log::error(e.what());
        result = -6;
    }
    catch (const CommandStringException &e)
    {
        Log::error(e.what());
        result = -7;
    }
    catch (const ProcessException &e)
    {
        Log::error(e.what());
        result = -8;
    }
    catch (const EmptyLinkObject &e)
    {
        Log::error(e.what());
        result = -9;
    }
    catch (const TargetIdTypeException &e)
    {
        Log::error(e.what());
        result = -10;
    }
    catch (const std::runtime_error &e)
    {
        Log::error("Unknown error:", e.what());
        result = -100;
    }
    catch (...)
    {
        Log::error("Unknown exception");
        result = -101;
    }

    processor->waitForFinished();
    project->logCommandTree();

    const auto end = std::chrono::steady_clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    Log::information("gibs took:", duration, "ms of your time");

    if (result == 0)
    {
        Log::information("gibs finished successfully!");
    }
    else
    {
        Log::error("gibs finished with errors! Error code:", result);
    }

    return result;
}
