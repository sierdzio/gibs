#pragma once

#include "stringlist.h"

#include <logger/log.h>

#include <set>
#include <string>

class CommandLine
{
  public:
    static StringList toStringList(int argc, char *argv[]);

    CommandLine(const StringList &args);

    std::string parsedFlagsText() const;

    std::string helpText() const;
    const std::string &versionText() const;

    const std::string &input() const;

    const std::string &logFilePath() const;
    bool isLogFilePathSet() const;
    Log::Type logLevel() const;

    bool isValid() const;
    bool hasHelp() const;
    bool hasVersion() const;
    bool runImmediately() const;
    bool isDebug() const;
    bool isQuickMode() const;
    bool colorfulLogs() const;
    bool isDryRun() const;
    bool isLogProcessOutput() const;

  private:
    struct ParseStatus
    {
        std::string current;
        std::string previous;
        std::set<std::string> parsed;
        bool hasError = false;
        bool isLastArgument = false;
    };

    bool parse();
    bool handleHelpAndVersion(ParseStatus &status);
    bool handleFlags(ParseStatus &status);
    bool handleOptionsWithValues(ParseStatus &status);
    bool handlePositionalArguments(ParseStatus &status);
    bool set(auto &value, const auto &toSet, ParseStatus &status,
             const std::string &name) const;

    [[nodiscard]] std::string helpAppend(std::string &&string, const StringList &flags,
                                         const std::string &explanation) const;

    StringList _args;
    std::string _executable;
    std::string _input;
    std::string _logFilePath;
    Log::Type _logLevel = Log::Type::Information;

    bool _isValid = false;
    bool _hasHelp = false;
    bool _hasVersion = false;
    bool _runImmediately = false;
    bool _isDebug = false;
    bool _isQuick = false;
    bool _colorfulLogs = true;
    bool _dryRun = false;
    bool _logProcessOutput = false;
};
