#include "tools.h"

#include <algorithm>

Tools::ScopeGuard::ScopeGuard(const std::function<void()> &function) : _function(function)
{
    // Nothing
}

Tools::ScopeGuard::~ScopeGuard()
{
    _function();
}

std::string Tools::removeQuotes(const std::string &path)
{
    std::string result = path;

    if (result.ends_with('\"'))
    {
        result.pop_back();
    }

    if (result.starts_with('\"'))
    {
        result = result.substr(1);
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
    constexpr auto listSep = ", ";

    std::string result;

    for (const auto &current : list)
    {
        if (not result.empty())
        {
            result.append(listSep);
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
    return value ? "true" : "false";
}
