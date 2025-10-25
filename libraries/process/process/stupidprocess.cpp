#include "stupidprocess.h"

#include <chrono>
#include <iostream>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

Exit StupidProcess::performAction()
{
    Exit result;

    std::cout << "Executing process: " << executable()
              << " with args: " << argsToString(arguments()) << std::endl;

    for (const auto i : std::views::iota(1, 6))
    {
        std::cout << "  -> Process: " << executable() << " iteration: " << i << std::endl;
        std::this_thread::sleep_for(1s);
    }

    std::cout << "Done! Process has finished: " << executable() << std::endl;

    return result;
}