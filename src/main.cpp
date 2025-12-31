//i executable name gibs

#include <chrono>
#include <iostream>
#include <stdexcept>

#include "exceptions/commanddepthexception.h"
#include "exceptions/commandexception.h"
#include "exceptions/commandnotfound.h"
#include "exceptions/emptylinkobject.h"
#include "parsing/parser.h"
#include "processing/processor.h"
#include "project/project.h"
#include "tools/commandline.h"

#include <logger/log.h>

int main(int argc, char *argv[])
{
    const auto begin = std::chrono::steady_clock::now();

    const CommandLine cmd(CommandLine::toStringList(argc, argv));

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

    if (cmd.hasVersion())
    {
        std::cout << cmd.versionText() << std::endl;
        return 0;
    }

    Log::setUseColorfulLogs(cmd.colorfulLogs());
    Log::information("Setting log level to:", Log::typeString(cmd.logLevel()));
    Log::setLogLevel(cmd.logLevel());
    Log::debug(cmd.parsedFlagsText());

    auto processor = std::make_shared<Processor>();
    processor->setDryRun(cmd.isDryRun());
    processor->setLogProcessOutput(cmd.isLogProcessOutput());

    auto project = std::make_shared<Project>(processor);

    Parser parser(cmd.input(), cmd.isQuickMode(), project);

    if (parser.status() != AppError::NoError)
    {
        return static_cast<int>(parser.status());
    }

    try
    {
        parser.parse();

        if (parser.status() != AppError::NoError)
        {
            return static_cast<int>(parser.status());
        }
    }
    catch (const CommandNotFound &e)
    {
        Log::error(e.what());
    }
    catch (const CommandDepthException &e)
    {
        // Warning because it is a missing functionality but not crucial
        Log::warning(e.what());
    }
    catch (const CommandException &e)
    {
        Log::error(e.what());
    }
    catch (const CommandStringException &e)
    {
        Log::error(e.what());
    }
    catch (const EmptyLinkObject &e)
    {
        Log::error(e.what());
    }
    catch (const std::runtime_error &e)
    {
        Log::error("Unknown error:", e.what());
    }
    catch (...)
    {
        Log::error("Unhandled exception");
    }

    processor->waitForFinished();
    project->logCommandTree();

    const auto end = std::chrono::steady_clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    Log::information("gibs took:", duration, "ms of your time");

    return 0;
}
