#pragma once

#include <vector>
#include <string>
class CommandLine
{
public:
    CommandLine(int argc, char *argv[]);

    bool isValid() const;
    bool hasHelp() const;
    bool hasVersion() const;
    bool runImmediately() const;

private:
    bool parse();

    std::vector<std::string> _args;
    bool _isValid = false;
    bool _hasHelp = false;
    bool _hasVersion = false;
    bool _runImmediately = false;
};