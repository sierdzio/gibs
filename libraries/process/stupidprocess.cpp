#include "stupidprocess.h"

#include <iostream>
#include <string>
#include <vector>

std::string argsToString(const std::vector<std::string> &args)
{
    std::string result;

    for (const auto &arg : args)
    {
        if (not result.empty())
        {
            result.append(" ");
        }

        result.append(arg);
    }

    return result;
}

void StupidProcess::setExecutable(const std::string &filePath)
{
    this->filePath = filePath;
}

void StupidProcess::setArguments(const std::vector<std::string> &args)
{
    this->args = args;
}

void StupidProcess::execute()
{
    std::cout << "Executing process: " << filePath << " with args: " << argsToString(args)
              << std::endl;
}