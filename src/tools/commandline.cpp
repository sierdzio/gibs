#include "commandline.h"
#include "log.h"
#include "versioninfo.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

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
constexpr auto DebugExplanation =
    "Compile in debug mode. By default, gibs compiles release binaries.";
constexpr auto Q = "-q";
constexpr auto Quick = "--quick";
constexpr auto QuickExplanation =
    "'Convention over configuration' mode - parse files only up to first line of "
    "'concrete code'. Do not check file checksums when doing incremental builds.";
constexpr auto Verbose = "--verbose";
constexpr auto VerboseExplanation =
    "Sets log level to 'Verbose'. "
    "If more than one log level is specified, or log level "
    "is combined with --verbose, only the last flag is "
    "taken nto account";

constexpr auto L = "-l";
constexpr auto LogLevel = "--log-level";
// TODO: use the X macro to list all log levels automatically
constexpr auto LogLevelExplanation =
    "Sets log level to one of: silent, error, warning, information, debug, verbose. "
    "Logs are printed for selected level and all levels above it. For example, "
    "when information is set, all error, warning and information logs will be "
    "printed, but no debug or verbose ones. 'silent' setting will not print any "
    "logs at all. Log level parser is case-sentitive, please make sure to provide "
    "log levels in lower case. "
    "If more than one log level is specified, or log level "
    "is combined with --verbose, only the last flag is "
    "taken nto account";

constexpr auto NoColor = "--no-color";
constexpr auto NoColorExplanation = "Disables color in log messages.";

constexpr auto DoubleSpace = "  ";
constexpr auto Quote = "\"";
}; // namespace

std::vector<std::string> CommandLine::toStringList(int argc, char *argv[])
{
    Log::verbose("Arg. count:", argc, "args:", std::string(*argv));

    std::vector<std::string> result;

    std::string multipart;

    for (int i = 0; i < argc; ++i)
    {
        std::string current(argv[i]);

        const auto quoteIndex = current.find(Quote);

        if (not multipart.empty() or quoteIndex != std::string::npos)
        {
            if (not multipart.empty())
            {
                multipart.push_back(' ');
            }

            if (quoteIndex != std::string::npos)
            {
                current.erase(quoteIndex, 1);
            }

            multipart.append(current);

            if (current.ends_with(Quote))
            {
                result.push_back(multipart);
                multipart.clear();
            }
        }
        else
        {
            result.push_back(argv[i]);
        }
    }

    if (not multipart.empty())
    {
        result.push_back(multipart);
    }

    return result;
}

CommandLine::CommandLine(const std::vector<std::string> &args) : _args(args)
{
    _isValid = parse();
}

std::string CommandLine::parsedFlagsText() const
{
    std::string result = "Set flags:\n";

    const auto appendIf = [](std::string *result, const bool shouldAppend,
                             const std::string &flag, const std::string &extraValue = {})
    {
        if (shouldAppend)
        {
            result->append(DoubleSpace);
            result->append(flag);

            if (not extraValue.empty())
            {
                result->append(": ");
                result->append(extraValue);
            }

            result->push_back('\n');
        }
    };

    appendIf(&result, runImmediately(), Run);
    appendIf(&result, isDebug(), Debug);
    appendIf(&result, isQuickMode(), Quick);
    appendIf(&result, true, LogLevel, Log::typeString(_logLevel));
    appendIf(&result, not colorfulLogs(), NoColor);

    return result;
}

std::string CommandLine::helpText() const
{
    std::string result;

    result.append(
        "C++ in-source project builder. Compile your projects without all the hassle "
        "connected with preparing a project file. Just run 'gibs main.cpp' and enjoy "
        "your "
        "compiled binary! More info: https://github.com/sierdzio/gibs\n\nOptions:\n");
    result = helpAppend(std::move(result), {H, Help}, HelpExplanation);
    result = helpAppend(std::move(result), {V, Version}, VersionExplanation);
    result = helpAppend(std::move(result), {R, Run}, RunExplanation);
    result = helpAppend(std::move(result), {D, Debug}, DebugExplanation);
    result = helpAppend(std::move(result), {Q, Quick}, QuickExplanation);
    result = helpAppend(std::move(result), {Verbose}, VerboseExplanation);
    result = helpAppend(std::move(result), {L, LogLevel}, LogLevelExplanation);
    result = helpAppend(std::move(result), {NoColor}, NoColorExplanation);

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

Log::Type CommandLine::logLevel() const
{
    return _logLevel;
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

bool CommandLine::isQuickMode() const
{
    return _isQuick;
}

bool CommandLine::colorfulLogs() const
{
    return _colorfulLogs;
}

bool CommandLine::parse()
{
    const auto canAdvance = [](const std::size_t i, const std::size_t size) -> bool
    { return i < size; };

    const auto size = _args.size();

    bool logLevelAlreadySet = false;
    std::string holdOverArgument;

    // Check if version or health flag is present
    for (std::size_t i = 0; canAdvance(i, size); ++i)
    {
        const auto &current = _args.at(i);

        // Handle held over options with values:

        if (holdOverArgument == LogLevel)
        {
            const auto value = Log::typeValue(current);

            if (logLevelAlreadySet)
            {
                Log::warning(
                    "Log level has already been set:", Log::typeString(_logLevel),
                    "overwriting with:", Log::typeString(value));
            }

            holdOverArgument.clear();
            logLevelAlreadySet = true;
            _logLevel = value;
            continue;
        }

        // Handle simple options (flags):

        if (current == H or current == Help)
        {
            _hasHelp = true;
            continue;
        }

        if (current == V or current == Version)
        {
            _hasVersion = true;
            continue;
        }

        if (current == R or current == Run)
        {
            _runImmediately = true;
            continue;
        }

        if (current == D or current == Debug)
        {
            _isDebug = true;
            continue;
        }

        if (current == Q or current == Quick)
        {
            _isQuick = true;
            continue;
        }

        if (current == Verbose)
        {
            if (logLevelAlreadySet)
            {
                Log::warning(
                    "Log level has already been set:", Log::typeString(_logLevel),
                    "overwriting with:", Log::typeString(Log::Type::Verbose));
            }

            logLevelAlreadySet = true;
            _logLevel = Log::Type::Verbose;
            continue;
        }

        if (current == NoColor)
        {
            _colorfulLogs = false;
            continue;
        }

        // Handle options with values:

        if (current == L || current == LogLevel)
        {
            holdOverArgument = LogLevel;
            continue;
        }

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

    string.append(DoubleSpace);

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
