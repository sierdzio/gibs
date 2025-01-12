#include "tools.h"

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
