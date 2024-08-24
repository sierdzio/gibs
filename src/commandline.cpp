#include "commandline.h"

#include <iostream>
#include <vector>

namespace {
    constexpr auto H = "-h";
    constexpr auto Help = "--help";
    constexpr auto HelpExplanation = "Displays this help information and exits.";
    constexpr auto V = "-v";
    constexpr auto Version = "--version";
    constexpr auto VersionExplanation = "Displays gibs version info and exits.";
    constexpr auto R = "-r";
    constexpr auto Run = "--run";
    constexpr auto RunExplanation = "Run the executable immediately after building.";
    constexpr auto D = "-d";
    constexpr auto Debug = "--debug";
    constexpr auto RunExplanation = "Compile in debug mode. By default, gibs compiles release binaries.";
};

CommandLine::CommandLine(int argc, char *argv[])
{
    std::cout << "Arg. count: " << argc << " args: " << std::string(*argv) << std::endl;

    std::string exe_name;
    std::string word;

    for (int i = 0; i < argc; ++i) {
        const auto &current = argv[i];
        if (current && (*current == ' ')) {
            _args.push_back(word);
            std::cout << "Found a word:" << word;
            word.clear();            
        }
    }   

    _isValid = parse(); 
}

bool CommandLine::isValid() const
{
    return _isValid;
}

bool CommandLine::hasHelp() const
{
    return _hasHelp;
}

bool CommandLine::hasVersion() const
{
    return _hasVersion;
}

bool CommandLine::runImmediately() const
{
    return _runImmediately;
}

bool CommandLine::isDebug() const
{
    return _isDebug;
}

bool CommandLine::parse()
{
    // Check if version or health flag is present
    for (std::size_t i = 0; i < _args.size(); ++i) {
        const auto &current = _args.at(i);

        if (current == H || current == Help) {
            _hasHelp = true;
            continue;
        }

        if (current == V || current == Version) {
            _hasVersion = true;
            continue;
        }

        if (current == R || current == Run) {
            _runImmediately = true;
            continue;
        }

        if (current == D || current == Debug) {
            _isDebug = true;
            continue;
        }
    }
    
    return true;
}
