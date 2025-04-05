#include "tools.h"
#include "parsing/syntax.h"

#include <algorithm>
#include <filesystem>

namespace
{
constexpr auto Quote = '"';
constexpr auto ListSep = ", ";
constexpr auto True = "true";
constexpr auto False = "false";
} // namespace

Tools::ScopeGuard::ScopeGuard(const std::function<void()> &function) : _function(function)
{
    // Nothing
}

Tools::ScopeGuard::~ScopeGuard()
{
    _function();
}

std::string Tools::prepareIncludePath(const std::string &input)
{
    // Immediate return if path is correct and does not need cleaning
    if (not input.starts_with(Syntax::CppKeywords::OpenLibraryInclude) and
        not input.starts_with(Quote))
    {
        return input;
    }

    auto result = input;

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

bool Tools::contains(const std::vector<std::string> &list, const std::string &string)
{
    return std::find(list.cbegin(), list.cend(), string) != list.cend();
}

bool Tools::contains(const std::string &string, const std::string &toFind)
{
    // TODO: C++23 use contains()
    return string.find(toFind) != std::string::npos;
}

std::string Tools::listToString(const std::vector<std::string> &list)
{
    std::string result;

    for (const auto &current : list)
    {
        if (not result.empty())
        {
            result.append(ListSep);
        }

        result.append(current);
    }

    return result;
}

std::string Tools::inBrackets(const std::string &string)
{
    return '(' + string + ')';
}

std::string Tools::boolToString(const bool value)
{
    return value ? True : False;
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
