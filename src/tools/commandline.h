#pragma once

#include "argumentslist.h"
#include "stringlist.h"

#include <logger/log.h>

#include <set>
#include <string>
#include <string_view>

class CommandLine
{
  public:
    static StringList toStringList(int argc, char *argv[]);

    CommandLine(const StringList &args);

    std::string parsedFlagsText() const;

    std::string helpText() const;
    const std::string &versionText() const;

    const std::string &input() const;
    std::string compilerSet() const;

    const std::string &logFilePath() const;
    bool isLogFilePathSet() const;
    Log::Type logLevel() const;

    ArgumentsList otherArguments() const;

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
    enum class HelpTopic
    {
        None,
        Commands,
        BuiltInFunctions
    };

    struct ParseStatus
    {
        std::string current;
        std::string previous;
        std::set<std::string> parsed;
        bool hasError = false;
        bool isFirstArgument = false;
        bool isLastArgument = false;
        bool isParsingOtherArguments = false;
    };

    bool parse();
    bool handleHelpAndVersion(ParseStatus &status);
    bool handleFlags(ParseStatus &status);
    bool handleOptionsWithValues(ParseStatus &status);
    bool handlePositionalArguments(ParseStatus &status);
    bool set(auto &value, const auto &toSet, ParseStatus &status,
             std::string_view name) const;
    bool isFlag(const ParseStatus &status) const;

    std::string otherArgumentsText() const;

    StringList _args;
    std::string _executable;
    std::string _input;
    std::string _compilerSet;
    std::string _logFilePath;
    ArgumentsList _otherArguments;
    Log::Type _logLevel = Log::Type::Information;
    HelpTopic _helpTopic = HelpTopic::None;

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
