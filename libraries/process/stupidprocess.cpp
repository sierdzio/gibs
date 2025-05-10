#include "stupidprocess.h"

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
    // Sth.
}