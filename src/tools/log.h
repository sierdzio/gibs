#pragma once

#include <array>
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

// TODO: add colors to logs

namespace Log
{
#define X(key, name) key,
enum class Type
{
    LOG_TYPES
};
#undef X

#define X(key, name) name,
constexpr std::array typeStrings = {LOG_TYPES};
#undef X

const std::string typeString(const Type type);
Type typeValue(const std::string &string);

void setLogLevel(const Type type);
bool isWithinLogLevel(const Type type);

std::string type(const Type type);

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

    constexpr auto Space = ' ';

    std::cout << Log::type(type) << Space;

    // This is a "loop" lambda
    ([&] { std::cout << args << Space; }(), ...);

    std::cout << std::endl;
}
}; // namespace Log
