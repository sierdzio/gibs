#pragma once

#include <iostream>
#include <string>
#include <vector>

#define LOG_TYPES                                                                        \
    X(Silent, "silent")                                                                  \
    X(Error, "error")                                                                    \
    X(Warning, "warning")                                                                \
    X(Information, "information")                                                        \
    X(Debug, "debug")                                                                    \
    X(Verbose, "verbose")

std::ostream &operator<<(std::ostream &stream,
                         const std::vector<std::string> &stringList);

namespace Log
{
#define X(key, name) key,
enum class Type
{
    LOG_TYPES
};
#undef X

// TODO: add catches for exceptions in places where these throwers are used,
// or catch and handle the errors in-place. No need to propagate the problem perhaps?
const std::string typeString(const Type type);
Type typeValue(const std::string &string);

void setLogLevel(const Type type);
Log::Type logLevel();
size_t logLevelsCount();
bool isWithinLogLevel(const Type type);

void setUseColorfulLogs(const bool enableColor);
bool usingColorfulLogs();

std::string beginning(const Type type);
std::string ending(const Type type);

template <typename... Types> void verbose(const Types &...args)
{
    log(Type::Verbose, args...);
}

template <typename... Types> void debug(const Types &...args)
{
    log(Type::Debug, args...);
}

template <typename... Types> void information(const Types &...args)
{
    log(Type::Information, args...);
}

template <typename... Types> void warning(const Types &...args)
{
    log(Type::Warning, args...);
}

template <typename... Types> void error(const Types &...args)
{
    log(Type::Error, args...);
}

template <typename... Types> void log(const Type type, const Types &...args)
{
    if (not isWithinLogLevel(type))
    {
        return;
    }

    std::cout << beginning(type);

    // This is a "loop" lambda
    ([&] { std::cout << args << ' '; }(), ...);

    std::cout << ending(type);
}
}; // namespace Log
