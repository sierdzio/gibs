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

    bool contains(const std::vector<std::string>& list, const std::string& string);
    bool contains(const std::string& string, const std::string& toFind);
};
