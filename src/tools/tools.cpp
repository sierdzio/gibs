#include "tools.h"
#include "parsing/syntax.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace
{
constexpr auto Quote = '\"';
constexpr std::string_view ListSep = ", ";
constexpr std::string_view True = "true";
constexpr std::string_view False = "false";
} // namespace

Tools::ScopeGuard::ScopeGuard(const std::function<void()> &function) : _function(function)
{
    // Nothing
}

Tools::ScopeGuard::~ScopeGuard()
{
    _function();
}

std::string Tools::prepareIncludePath(const std::string_view input)
{
    // Immediate return if path is correct and does not need cleaning
    if (not input.starts_with(Syntax::CppKeywords::OpenLibraryInclude) and
        not input.starts_with(Quote))
    {
        return std::string(input);
    }

    auto result = std::string(input);

    if (result.starts_with(Syntax::CppKeywords::OpenLibraryInclude) or
        result.starts_with(Quote))
    {
        result.erase(0, 1);
    }

    if (result.ends_with(Syntax::CppKeywords::CloseLibraryInclude) or
        result.ends_with(Quote))
    {
        result.pop_back();
    }

    return result;
}

bool Tools::contains(const StringList &list, const std::string &string)
{
    return std::find(list.cbegin(), list.cend(), string) != list.cend();
}

bool Tools::contains(const std::string &string, const std::string &toFind)
{
    return string.contains(toFind);
}

bool Tools::contains(const std::string_view &string, const std::string_view &toFind)
{
    return string.contains(toFind);
}

std::string Tools::listToString(const StringList &list)
{
    std::string result;

    for (const auto &current : list)
    {
        if (not result.empty() and not current.empty())
        {
            result.append(ListSep);
        }

        result.append(current);
    }

    return result;
}

std::string Tools::inBrackets(const std::string_view string)
{
    return '(' + std::string(string) + ')';
}

std::string Tools::inSquareBrackets(const std::string_view string)
{
    return '[' + std::string(string) + ']';
}

std::string Tools::inQuotes(const std::string_view string)
{
    return Quote + std::string(string) + Quote;
}

std::string Tools::boolToString(const bool value)
{
    return std::string(value ? True : False);
}

bool Tools::isPathToFile(const std::string &path)
{
    // TODO: these checks and results should be cached!
    const std::filesystem::path rawPath(path);
    return std::filesystem::is_regular_file(rawPath) and
           not std::filesystem::is_directory(rawPath);
}

bool Tools::isHeaderFile(const std::string &path)
{
    // TODO: these checks and results should be cached!
    return path.ends_with(Syntax::Extension::HeaderFile1) or
           path.ends_with(Syntax::Extension::HeaderFile2) or
           path.ends_with(Syntax::Extension::HeaderFile3);
}

bool Tools::isWhitespace(const char character)
{
    // TODO: also consider narrow space, non-breaking space etc.
    return std::isspace(static_cast<unsigned char>(character));
}

StringList Tools::pathsToStrings(const std::vector<std::filesystem::path> &paths)
{
    StringList result;

    for (const auto &path : paths)
    {
        result.emplace_back(path.string());
    }

    return result;
}

std::string Tools::toLower(const std::string &input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](const unsigned char character)
                   { return static_cast<char>(std::tolower(character)); });
    return result;
}

unsigned int Tools::terminalWidth()
{
#if defined(_WIN32)
    const auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle != INVALID_HANDLE_VALUE)
    {
        CONSOLE_SCREEN_BUFFER_INFO info{};
        if (GetConsoleScreenBufferInfo(handle, &info))
        {
            const auto width = info.srWindow.Right - info.srWindow.Left + 1;
            if (width > 0)
            {
                return static_cast<unsigned int>(width);
            }
        }
    }
#else
    struct winsize windowSize{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &windowSize) == 0 && windowSize.ws_col > 0)
    {
        return static_cast<unsigned int>(windowSize.ws_col);
    }
#endif

    return 80;
}
