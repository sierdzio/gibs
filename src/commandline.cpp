#include "commandline.h"
#include "versioninfo.h"
#include "log.h"

#include <string>
#include <vector>
#include <cassert>
namespace
{
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
    constexpr auto DebugExplanation = "Compile in debug mode. By default, gibs compiles release binaries.";
};

CommandLine::CommandLine(int argc, char *argv[])
{
    Log::information("Arg. count:", argc, "args:", std::string(*argv));

    for (int i = 0; i < argc; ++i)
    {
        const auto current = std::string(argv[i]);
        _args.push_back(current);
        Log::debug("Found a word:", current);
    }

    _isValid = parse();
}

std::string CommandLine::helpText() const
{
    std::string result;

    result.append("C++ in-source project builder. Compile your projects without all the hassle "
                  "connected with preparing a project file. Just run 'gibs main.cpp' and enjoy your "
                  "compiled binary! More info: https://github.com/sierdzio/gibs\n\nOptions:\n");
    result = helpAppend(std::move(result), {H, Help}, HelpExplanation);
    result = helpAppend(std::move(result), {V, Version}, VersionExplanation);
    result = helpAppend(std::move(result), {R, Run}, RunExplanation);
    result = helpAppend(std::move(result), {D, Debug}, DebugExplanation);

    return result;
}

std::string CommandLine::versionText() const
{
    return VersionInfo::versionNumber;
}

std::string CommandLine::input() const
{
    return _input;
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
    for (std::size_t i = 0; i < _args.size(); ++i)
    {
        const auto &current = _args.at(i);

        // Handle simple options (flags):

        if (current == H || current == Help)
        {
            _hasHelp = true;
            continue;
        }

        if (current == V || current == Version)
        {
            _hasVersion = true;
            continue;
        }

        if (current == R || current == Run)
        {
            _runImmediately = true;
            continue;
        }

        if (current == D || current == Debug)
        {
            _isDebug = true;
            continue;
        }

        // TODO: handle options with values

        // Handle positional arguments:
        _input = current;
    }

    return true;
}

std::string CommandLine::helpAppend(std::string &&string,
                                    const std::vector<std::string> &flags,
                                    const std::string &explanation) const
{
    assert(flags.size() > 0);

    string.push_back(' ');

    bool isFirst = true;
    for (const auto &flag : flags)
    {
        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            string.append(std::string(", "));
        }

        string.append(flag);
    }

    string.push_back('\t');
    string.append(explanation);
    string.push_back('\n');

    return std::move(string);
}
