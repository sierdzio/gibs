#ifndef LOG_H
#define LOG_H
#pragma once

#include <string>

class Log
{
public:
    explicit Log();

    static void log(const std::string &message);
};

#endif // LOG_H
