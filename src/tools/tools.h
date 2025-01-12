#pragma once

#include <functional>
#include <string>

namespace Tools {
    class ScopeGuard {
        public:
            ScopeGuard(const std::function<void()> &function);
            ~ScopeGuard();

        private:
            std::function<void()> _function;
    };

    std::string removeQuotes(const std::string& path);
};
