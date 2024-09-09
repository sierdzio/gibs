#include "tools.h"

Tools::ScopeGuard::ScopeGuard(const std::function<void()> &function) : _function(function)
{
    // Nothing
}

Tools::ScopeGuard::~ScopeGuard()
{
    _function();
}
