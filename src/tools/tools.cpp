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

std::string Tools::removeQuotes(const std::string& path)
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

bool Tools::contains(const std::vector<std::string>& list, const std::string& string)
{
    return std::find(list.cbegin(), list.cend(), string) != list.cend();
}

bool Tools::contains(const std::string& string, const std::string& toFind)
{
    // TODO: C++23 use contains()
    return string.find(toFind) != std::string::npos;
}
