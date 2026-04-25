#pragma once

#include "color.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef SYNCHRONISE_LOG_MESSAGES
#include <mutex>
#endif

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
Log::Type typeValue(const std::string &string);

void setLogLevel(const Type type);
Log::Type logLevel();
size_t logLevelsCount();
bool isWithinLogLevel(const Type type);

void setUseColorfulLogs(const bool enableColor);
bool usingColorfulLogs();

void setLogFile(const std::string &path);
void closeLogFile();

namespace Private
{
std::string beginning(const Type type, const Color &color = {});
std::string ending(const Type type, const Color &color = {});

bool isFileLoggingEnabled();
std::ostream &logFileStream();

#ifdef SYNCHRONISE_LOG_MESSAGES
static std::mutex PrintMutex;
#endif
} //namespace Private

template <typename... Types> void log(const Type type, const Types &...args)
{
    log(type, Color(), args...);
}

template <typename... Types>
void log(const Type type, const Color &color, const Types &...args)
{
    if (not isWithinLogLevel(type))
    {
        return;
    }

#ifdef SYNCHRONISE_LOG_MESSAGES
    std::lock_guard<std::mutex> guard(Private::PrintMutex);
#endif

    std::ostringstream message;
    message << Private::beginning(type, color);
    ([&] { message << args << ' '; }(), ...);
    message << Private::ending(type, color);

    const auto output = message.str();
    std::cout << output;

    if (Private::isFileLoggingEnabled())
    {
        Private::logFileStream() << output;
        Private::logFileStream().flush();
    }
}

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

template <typename... Types> void verbose(const Color &color, const Types &...args)
{
    log(Type::Verbose, color, args...);
}

template <typename... Types> void debug(const Color &color, const Types &...args)
{
    log(Type::Debug, color, args...);
}

template <typename... Types> void information(const Color &color, const Types &...args)
{
    log(Type::Information, color, args...);
}

template <typename... Types> void warning(const Color &color, const Types &...args)
{
    log(Type::Warning, color, args...);
}

template <typename... Types> void error(const Color &color, const Types &...args)
{
    log(Type::Error, color, args...);
}

std::ostream &operator<<(std::ostream &stream, const Log::Type type);

}; // namespace Log
