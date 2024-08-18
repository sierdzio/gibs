#include "commandline.h"

#include <iostream>

CommandLine::CommandLine(int argc, char *argv[])
{
    std::cout << "Arg. count: " << argc << " args: " << argv << std::endl;
}
