#include "stupidprocess.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

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

    std::this_thread::sleep_for(5s);

    std::cout << "Done! Process has finished: " << filePath << std::endl;
}