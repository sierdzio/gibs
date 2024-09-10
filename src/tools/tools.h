#pragma once

#include <functional>

namespace Tools {
    class ScopeGuard {
        public:
            ScopeGuard(const std::function<void()> &function);
            ~ScopeGuard();

        private:
            std::function<void()> _function;
    };
};
