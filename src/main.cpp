#include <iostream>

#include "tools/commandline.h"
#include "parsing/parser.h"
#include "tools/log.h"

int main(int argc, char *argv[])
{
    const CommandLine cmd(argc, argv);

    if (cmd.hasHelp()) {
        std::cout << cmd.helpText() << std::endl;
        return 0;
    }

    if (cmd.hasVersion()) {
        std::cout << cmd.versionText() << std::endl;
        return 0;
    }

    Log::information("Setting log level to:", Log::typeString(cmd.logLevel()));
    Log::setLogLevel(cmd.logLevel());

    Parser parser(&cmd);

    if (parser.status() != AppError::NoError) {
        return static_cast<int>(parser.status());
    }

    parser.parse();

    if (parser.status() != AppError::NoError) {
        return static_cast<int>(parser.status());
    }

    return 0;
}
