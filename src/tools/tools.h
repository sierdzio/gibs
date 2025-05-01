#pragma once

#include <functional>
#include <string>
#include <vector>

namespace Tools
{
class ScopeGuard
{
  public:
    ScopeGuard(const std::function<void()> &function);
    ~ScopeGuard();

  private:
    std::function<void()> _function;
};

/*!
  Transforms C++ include statement paths into simple path strings, for example:

  Input: <library/include.h>
  Output: library/include.h

  Or:

  Input: "local/include.h"
  Output: local/include.h

  \note This function does not verify the actual path in any way, nor does it
  check if the file pointed to exists.
*/
std::string prepareIncludePath(const std::string &input);

bool contains(const std::vector<std::string> &list, const std::string &string);
bool contains(const std::string &string, const std::string &toFind);

std::string listToString(const std::vector<std::string> &list);
std::string inBrackets(const std::string &string);
std::string boolToString(const bool value);

bool isPathToFile(const std::string &path);
bool isHeaderFile(const std::string &path);

bool isWhitespace(const char character);
}; // namespace Tools
