#include "log.h"

#include <iostream>

Log::Log()
{
}

void Log::log(const std::string &message)
{
    std::cout << "L: " << message << std::endl;
}
