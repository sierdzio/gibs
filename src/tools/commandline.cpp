#include "commandline.h"
#include "helpdata.h"
#include "processing/compilerset.h"
#include "tools.h"
#include "versioninfo.h"

#include <cstring>
#include <logger/log.h>

#include <cassert>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

namespace
{
constexpr std::string_view H = "-h";
constexpr std::string_view Help = "--help";
constexpr std::string_view HelpExplanation = "Displays this help information and exits.";
constexpr std::string_view V = "-v";
constexpr std::string_view Version = "--version";
constexpr std::string_view VersionExplanation = "Displays gibs version info and exits.";
constexpr std::string_view R = "-r";
constexpr std::string_view Run = "--run";
constexpr std::string_view RunExplanation =
    "Run the executable immediately after building.";
constexpr std::string_view D = "-d";
constexpr std::string_view Debug = "--debug";
constexpr std::string_view DebugExplanation =
    "Compile in debug mode. By default, gibs compiles release binaries.";
constexpr std::string_view Q = "-q";
constexpr std::string_view Quick = "--quick";
constexpr std::string_view QuickExplanation =
    "'Convention over configuration' mode - parse files only up to first line of "
    "'concrete code'. Do not check file checksums when doing incremental builds.";
constexpr std::string_view Verbose = "--verbose";
constexpr std::string_view VerboseExplanation =
    "Sets log level to 'Verbose'. "
    "If more than one log level is specified, or log level "
    "is combined with --verbose, only the last flag is "
    "taken nto account";

constexpr std::string_view L = "-l";
constexpr std::string_view LogLevel = "--log-level";
// TODO: use the X macro to list all log levels automatically
constexpr std::string_view LogLevelExplanation =
    "Sets log level to one of: silent, error, warning, information, debug, verbose. "
    "Logs are printed for selected level and all levels above it. For example, "
    "when information is set, all error, warning and information logs will be "
    "printed, but no debug or verbose ones. 'silent' setting will not print any "
    "logs at all. Log level parser is case-sensitive, please make sure to provide "
    "log levels in lower case. "
    "If more than one log level is specified, or log level "
    "is combined with --verbose, only the last flag is "
    "taken into account.";

constexpr std::string_view LogFilePath = "--log-file-path";
constexpr std::string_view LogFilePathExplanation =
    "Duplicates all console logs into this file. "
    "If the file does not exist, it will be created. "
    "If the file does exist, it will be cleared and written to. "
    "If a path to a directory is provided, a log file called 'gibs-<datetime>.log "
    "will be created.";

constexpr std::string_view NoColor = "--no-color";
constexpr std::string_view NoColorExplanation = "Disables color in log messages.";

constexpr std::string_view DryRun = "--dry-run";
constexpr std::string_view DryRunExplanation =
    "Does not actually run any compilation or linking commands. Commands are only "
    "printed out but not executed.";

constexpr std::string_view LogProcessOutput = "--log-process-output";
constexpr std::string_view LogProcessOutputExplanation =
    "Prints standard and error outputs from spawned processes "
    "(compiler, linker etc.).";

constexpr std::string_view C = "-c";
constexpr std::string_view Compiler = "--compiler";
constexpr std::string_view CompilerSetOption = "--compiler-set";
constexpr std::string_view CompilerExplanation =
    "Selects the compiler set to use: gcc, clang, apple-clang. "
    "Defaults to apple-clang on macOS and gcc elsewhere.";

constexpr std::string_view Executable = "executable";
constexpr std::string_view Input = "input";
constexpr std::string_view InputExplanation =
    "Path to the input file or directory to build. If a directory is provided, gibs will "
    "look for files with supported extensions in it and its subdirectories. If a file is "
    "provided, gibs will try to build it. If no input is provided, gibs will try to "
    "build a file called 'main' with a supported extension in the current directory.";

constexpr std::string_view OtherArguments = "--";
constexpr std::string_view OtherArgumentsExplanation =
    "Other, user-defined arguments passed to the program should be placed after this "
    "separator. For example: 'gibs --main.cpp -- --my-flag' will enable feature "
    "'my-flag', and 'gibs --main.cpp -- --my-flag=OFF' will disable it when compiling "
    "main.cpp.";

constexpr std::string_view DoubleSpace = "  ";
constexpr std::string_view Quote = "\"";
constexpr std::string_view DateTimeFormat = "%Y-%m-%dT%H:%M:%SZ";
constexpr std::string_view NegativeOptionBeginning = "no-";

std::string toUpper(std::string string)
{
    for (auto &character : string)
    {
        character =
            static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }
    return string;
}
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
                             const std::string_view &flag,
                             const std::string_view &extraValue = {})
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
    appendIf(&result, true, CompilerSetOption, compilerSet());
    appendIf(&result, true, Input, input());
    appendIf(&result, true, OtherArguments, otherArgumentsText());

    return result;
}

std::string CommandLine::helpText() const
{
    HelpData help;

    help.addIntro(
        "C++ in-source project builder. Compile your projects without all the hassle "
        "connected with preparing a project file. Just run 'gibs main.cpp' and enjoy "
        "your compiled binary! More info: "
        "https://github.com/sierdzio/gibs\n\nOptions:\n");
    help.addEntry({H, Help}, HelpExplanation);
    help.addEntry({V, Version}, VersionExplanation);
    help.addEntry({R, Run}, RunExplanation);
    help.addEntry({D, Debug}, DebugExplanation);
    help.addEntry({Q, Quick}, QuickExplanation);
    help.addEntry({Verbose}, VerboseExplanation);
    help.addEntry({L, LogLevel}, LogLevelExplanation);
    help.addEntry({LogFilePath}, LogFilePathExplanation);
    help.addEntry({NoColor}, NoColorExplanation);
    help.addEntry({DryRun}, DryRunExplanation);
    help.addEntry({LogProcessOutput}, LogProcessOutputExplanation);
    help.addEntry({C, Compiler, CompilerSetOption}, CompilerExplanation);
    help.addEntry({Input}, InputExplanation);
    help.addEntry({OtherArguments}, OtherArgumentsExplanation);

    // TODO: make width dynamic based on terminal width
    return help.formatted(180);
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

std::string CommandLine::compilerSet() const
{
    if (not _compilerSet.empty())
    {
        return _compilerSet;
    }

    return CompilerSet::defaultForPlatform().name;
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

ArgumentsList CommandLine::otherArguments() const
{
    return _otherArguments;
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
        status.isLastArgument = (i == size - 1 or _args.at(i + 1) == OtherArguments);

        Tools::ScopeGuard guard([&status]() { status.previous = status.current; });

        if (status.current == OtherArguments)
        {
            status.isParsingOtherArguments = true;
            continue;
        }

        if (status.isParsingOtherArguments)
        {
            // TODO: move to function

            if (status.current.starts_with(OtherArguments))
            {
                const std::string rawName = status.current.substr(OtherArguments.size());
                const auto equalIndex = rawName.find('=');
                std::string name = rawName.substr(0, equalIndex);
                std::string value;

                if (equalIndex != std::string::npos)
                {
                    value = rawName.substr(equalIndex + 1);
                }

                bool isOn = true;
                bool isNegative = false;

                if (name.starts_with(NegativeOptionBeginning))
                {
                    isNegative = true;
                    name = name.substr(NegativeOptionBeginning.size());
                    isOn = false;
                }

                if (not value.empty())
                {
                    const auto normalizedValue = toUpper(value);

                    if (normalizedValue == "ON")
                    {
                        isOn = not isNegative;
                    }
                    else if (normalizedValue == "OFF")
                    {
                        isOn = isNegative ? true : false;
                    }
                    else
                    {
                        Log::error("Unrecognized value:", value,
                                   "for other argument:", status.current);
                        return false;
                    }
                }

                Log::verbose("Parsing other argument:", status.current,
                             "parsed name:", name, "isOn:", isOn);

                _otherArguments.emplace(name, isOn);
            }
            else
            {
                Log::error("Unrecognized other argument:", status.current);
                return false;
            }

            continue;
        }

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
    if (not status.parsed.contains(std::string(LogLevel)))
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

    if (status.current == Compiler or status.current == CompilerSetOption)
    {
        if (status.isLastArgument)
        {
            Log::error("Missing value for option:", status.current);
            status.hasError = true;
            return false;
        }

        return true;
    }

    if (status.previous == Compiler or status.previous == CompilerSetOption)
    {
        if (not CompilerSet::isKnownName(status.current))
        {
            Log::error("Unsupported compiler set:", status.current);
            status.hasError = true;
            return false;
        }

        return set(_compilerSet, status.current, status, CompilerSetOption);
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
            dateTimeStream << std::put_time(std::localtime(&time), DateTimeFormat.data());
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
    if (status.isFirstArgument and not status.parsed.contains(std::string(Executable)))
    {
        return set(_executable, status.current, status, Executable);
    }

    // Second positional argument is the input file
    if (not status.parsed.contains(std::string(Input)))
    {
        return set(_input, status.current, status, Input);
    }

    Log::error("Unrecognized positional command line argument:", status.current);
    return false;
}

bool CommandLine::set(auto &value, const auto &toSet, ParseStatus &status,
                      const std::string_view &name) const
{
    value = toSet;

    const std::string nameStr(name);

    if (name == LogLevel)
    {
        const auto result = status.parsed.insert(nameStr);

        if (not result.second)
        {
            Log::warning("Duplicated command line argument:", name, "with value:", toSet,
                         "(last value wins)");
            return true;
        }

        Log::verbose("Found argument:", name, "with value:", value);
        return true;
    }

    const auto result = status.parsed.insert(nameStr);

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

std::string CommandLine::otherArgumentsText() const
{
    std::string result;

    for (const auto &[name, value] : _otherArguments)
    {
        if (not result.empty())
        {
            result.append(", ");
        }

        result.append(Tools::inQuotes(name));
        result.append(" = ");

        std::string valueString;

        if (value.has_value())
        {
            if (value.type() == typeid(bool))
            {
                valueString = Tools::boolToString(std::any_cast<bool>(value));
            }
            else if (value.type() == typeid(std::string))
            {
                valueString = std::any_cast<std::string>(value);
            }
            else
            {
                result.append("unsupported value type");
            }

            result.append(Tools::inQuotes(valueString));
        }
    }

    return result;
}
