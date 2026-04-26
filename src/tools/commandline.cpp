#include "commandline.h"
#include "tools/tools.h"
#include "versioninfo.h"

#include <logger/log.h>

#include <cassert>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

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
    "logs at all. Log level parser is case-sensitive, please make sure to provide "
    "log levels in lower case. "
    "If more than one log level is specified, or log level "
    "is combined with --verbose, only the last flag is "
    "taken into account.";

constexpr auto LogFilePath = "--log-file-path";
constexpr auto LogFilePathExplanation =
    "Duplicates all console logs into this file. "
    "If the file does not exist, it will be created. "
    "If the file does exist, it will be cleared and written to. "
    "If a path to a directory is provided, a log file called 'gibs-<datetime>.log "
    "will be created.";

constexpr auto NoColor = "--no-color";
constexpr auto NoColorExplanation = "Disables color in log messages.";

constexpr auto DryRun = "--dry-run";
constexpr auto DryRunExplanation =
    "Does not actually run any compilation or linking commands. Commands are only "
    "printed out but not executed.";

constexpr auto LogProcessOutput = "--log-process-output";
constexpr auto LogProcessOutputExplanation =
    "Prints standard and error outputs from spawned processes "
    "(compiler, linker etc.).";

constexpr auto Executable = "executable";
constexpr auto Input = "input";
constexpr auto InputExplanation =
    "Path to the input file or directory to build. If a directory is provided, gibs will "
    "look for files with supported extensions in it and its subdirectories. If a file is "
    "provided, gibs will try to build it. If no input is provided, gibs will try to "
    "build a file called 'main' with a supported extension in the current directory.";

constexpr auto DoubleSpace = "  ";
constexpr auto Quote = "\"";
}; // namespace

StringList CommandLine::toStringList(int argc, char *argv[])
{
    Log::verbose("Arg. count:", argc, "args:", std::string(*argv));

    StringList result;

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
                result.emplace_back(multipart);
                multipart.clear();
            }
        }
        else
        {
            result.emplace_back(argv[i]);
        }
    }

    if (not multipart.empty())
    {
        result.emplace_back(multipart);
    }

    return result;
}

CommandLine::CommandLine(const StringList &args) : _args(args)
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
    appendIf(&result, isLogFilePathSet(), LogFilePath, logFilePath());
    appendIf(&result, not colorfulLogs(), NoColor);
    appendIf(&result, isDryRun(), DryRun);
    appendIf(&result, isLogProcessOutput(), LogProcessOutput);
    appendIf(&result, true, Input, input());

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
    result = helpAppend(std::move(result), {LogFilePath}, LogFilePathExplanation);
    result = helpAppend(std::move(result), {NoColor}, NoColorExplanation);
    result = helpAppend(std::move(result), {DryRun}, DryRunExplanation);
    result =
        helpAppend(std::move(result), {LogProcessOutput}, LogProcessOutputExplanation);
    result = helpAppend(std::move(result), {Input}, InputExplanation);

    return result;
}

const std::string &CommandLine::versionText() const
{
    return VersionInfo::versionNumber;
}

const std::string &CommandLine::input() const
{
    Log::warning("Input file:", _input);
    return _input;
}

const std::string &CommandLine::logFilePath() const
{
    return _logFilePath;
}

bool CommandLine::isLogFilePathSet() const
{
    return _logFilePath.size() > 0;
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

bool CommandLine::isDryRun() const
{
    return _dryRun;
}

bool CommandLine::isLogProcessOutput() const
{
    return _logProcessOutput;
}

bool CommandLine::parse()
{
    const auto size = _args.size();
    ParseStatus status;

    // Check if version or health flag is present
    for (std::size_t i = 0; i < size; ++i)
    {
        status.current = _args.at(i);
        status.isFirstArgument = (i == 0);
        status.isLastArgument = (i == size - 1);

        Tools::ScopeGuard guard([&status]() { status.previous = status.current; });

        if (status.previous == LogFilePath and isFlag(status))
        {
            if (handlePositionalArguments(status))
            {
                continue;
            }
        }

        const bool argumentValid = handleHelpAndVersion(status) or handleFlags(status) or
                                   handleOptionsWithValues(status) or
                                   handlePositionalArguments(status);

        if (not argumentValid)
        {
            Log::error("Unrecognized command line argument:", status.current);
            return false;
        }
    }

    // Set default log level:
    if (not status.parsed.contains(LogLevel))
    {
        set(_logLevel, Log::Type::Information, status, LogLevel);
    }

    return true;
}

bool CommandLine::handleHelpAndVersion(ParseStatus &status)
{
    if (status.hasError)
    {
        return false;
    }

    if (status.current == H or status.current == Help)
    {
        return set(_hasHelp, true, status, Help);
    }

    if (status.current == V or status.current == Version)
    {
        return set(_hasVersion, true, status, Version);
    }

    return false;
}

bool CommandLine::handleFlags(ParseStatus &status)
{
    if (status.hasError)
    {
        return false;
    }

    if (status.current == R or status.current == Run)
    {
        return set(_runImmediately, true, status, Run);
    }

    if (status.current == D or status.current == Debug)
    {
        return set(_isDebug, true, status, Debug);
    }

    if (status.current == Q or status.current == Quick)
    {
        return set(_isQuick, true, status, Quick);
    }

    if (status.current == Verbose)
    {
        return set(_logLevel, Log::Type::Verbose, status, LogLevel);
    }

    if (status.current == NoColor)
    {
        return set(_colorfulLogs, false, status, NoColor);
    }

    if (status.current == DryRun)
    {
        return set(_dryRun, true, status, DryRun);
    }

    if (status.current == LogProcessOutput)
    {
        return set(_logProcessOutput, true, status, LogProcessOutput);
    }

    return false;
}

bool CommandLine::handleOptionsWithValues(ParseStatus &status)
{
    if (status.hasError)
    {
        return false;
    }

    if (status.previous == LogLevel)
    {
        return set(_logLevel, Log::typeValue(status.current), status, LogLevel);
    }

    if (status.current == L || status.current == LogLevel)
    {
        return true;
    }

    return false;
}

bool CommandLine::handlePositionalArguments(ParseStatus &status)
{
    if (status.hasError)
    {
        return false;
    }

    Log::debug("handlePositionalArguments:", "current:", status.current,
               "previous:", status.previous, "isLastArgument:", status.isLastArgument);

    if (not status.isLastArgument and status.current == LogFilePath)
    {
        // LogFilePath is not the last argument, so we proceed to parse next arguments
        return true;
    }

    // Handle log file path value (previous was --log-file-path)
    if (status.previous == LogFilePath or
        (status.isLastArgument and status.current == LogFilePath))
    {
        if (isFlag(status))
        {
            const auto now = std::chrono::system_clock::now();
            const auto time = std::chrono::system_clock::to_time_t(now);
            std::ostringstream dateTimeStream;
            dateTimeStream << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%SZ");
            const auto dateTimeString = dateTimeStream.str();
            const auto path =
                std::filesystem::current_path() / ("gibs-" + dateTimeString + ".log");

            Log::information(
                "Log file path argument provided without a value, using default:", path);
            return set(_logFilePath, path, status, LogFilePath);
        }

        return set(_logFilePath, status.current, status, LogFilePath);
    }

    // First positional argument is the executable
    if (status.isFirstArgument and not status.parsed.contains(Executable))
    {
        return set(_executable, status.current, status, Executable);
    }

    // Second positional argument is the input file
    if (not status.parsed.contains(Input))
    {
        return set(_input, status.current, status, Input);
    }

    Log::error("Unrecognized positional command line argument:", status.current);
    return false;
}

bool CommandLine::set(auto &value, const auto &toSet, ParseStatus &status,
                      const std::string &name) const
{
    value = toSet;

    if (name == LogLevel)
    {
        const auto result = status.parsed.insert(name);

        if (not result.second)
        {
            Log::warning("Duplicated command line argument:", name, "with value:", toSet,
                         "(last value wins)");
            return true;
        }

        Log::verbose("Found argument:", name, "with value:", value);
        return true;
    }

    const auto result = status.parsed.insert(name);

    if (not result.second)
    {
        Log::warning("Duplicated command line argument:", name, "with value:", toSet);
        status.hasError = true;
        return false;
    }
    else
    {
        Log::verbose("Found argument:", name, "with value:", value);
    }

    return true;
}

bool CommandLine::isFlag(const ParseStatus &status) const
{
    return status.current.starts_with('-');
}

std::string CommandLine::helpAppend(std::string &&string, const StringList &flags,
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
    string.push_back('\t');
    string.append(explanation);
    string.push_back('\n');

    return std::move(string);
}
