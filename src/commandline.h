#pragma once

#include <vector>
#include <string>
class CommandLine
{
public:
    CommandLine(int argc, char *argv[]);

    std::string helpText() const;
    std::string versionText() const;

    bool isValid() const;
    bool hasHelp() const;
    bool hasVersion() const;
    bool runImmediately() const;
    bool isDebug() const;

private:
    bool parse();
    [[nodiscard]] std::string helpAppend(std::string &&string, 
                                         const std::vector<std::string> &flags,
                                         const std::string &explanation) const;

    std::vector<std::string> _args;
    bool _isValid = false;
    bool _hasHelp = false;
    bool _hasVersion = false;
    bool _runImmediately = false;
    bool _isDebug = false;
};