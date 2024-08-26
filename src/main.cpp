#include <iostream>
#include <string>

#include "log.h"
#include "commandline.h"

int main(int argc, char *argv[])
{
    Log::log("Hello gibs!");

    const CommandLine cmd(argc, argv);

    if (cmd.hasHelp()) {
        std::cout << cmd.helpText() << std::endl;
        return 0;
    }

    if (cmd.hasVersion()) {
        std::cout << cmd.versionText() << std::endl;
        return 0;
    }

    return 0;
}
