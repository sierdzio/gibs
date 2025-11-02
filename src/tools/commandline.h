#pragma once

#include "stringlist.h"

#include <logger/log.h>

#include <string>
#include <vector>

class CommandLine
{
  public:
    static StringList toStringList(int argc, char *argv[]);

    CommandLine(const StringList &args);

    std::string parsedFlagsText() const;

    std::string helpText() const;
    std::string versionText() const;

    std::string input() const;

    Log::Type logLevel() const;

    bool isValid() const;
    bool hasHelp() const;
    bool hasVersion() const;
    bool runImmediately() const;
    bool isDebug() const;
    bool isQuickMode() const;
    bool colorfulLogs() const;
    bool isDryRun() const;

  private:
    bool parse();
    [[nodiscard]] std::string helpAppend(std::string &&string, const StringList &flags,
                                         const std::string &explanation) const;

    StringList _args;
    std::string _input;
    Log::Type _logLevel = Log::Type::Information;

    bool _isValid = false;
    bool _hasHelp = false;
    bool _hasVersion = false;
    bool _runImmediately = false;
    bool _isDebug = false;
    bool _isQuick = false;
    bool _colorfulLogs = true;
    bool _dryRun = false;
};
