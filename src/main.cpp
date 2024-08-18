#include <string>

#include "log.h"
#include "commandline.h"

int main(int argc, char *argv[])
{
    Log::log("Hello gibs!");

    CommandLine cmdln(argc, argv);

    return 0;
}
