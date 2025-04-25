#include <chrono>
#include <iostream>

#include "exceptions/commanddepthexception.h"
#include "exceptions/commandnotfound.h"
#include "exceptions/emptylinkobject.h"
#include "parsing/parser.h"
#include "tools/commandline.h"
#include "tools/log.h"

int main(int argc, char *argv[])
{
    const auto begin = std::chrono::steady_clock::now();

    const CommandLine cmd(argc, argv);

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

    Log::information("Setting log level to:", Log::typeString(cmd.logLevel()));
    Log::setLogLevel(cmd.logLevel());
    Log::debug(cmd.parsedFlagsText());

    Parser parser(&cmd);

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
    catch (const EmptyLinkObject &e)
    {
        Log::error(e.what());
    }
    catch (...)
    {
        Log::error("Unhandled exception");
    }

    const auto end = std::chrono::steady_clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    Log::information("gibs took:", duration, "ms of your time");

    return 0;
}
