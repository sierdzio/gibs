#pragma once

#include "stringlist.h"

#include <filesystem>
#include <functional>

namespace Tools
{
/*!
 * Executes the provided function when going out of scope.
 *
 * Use whenever you need to ensure that a certain piece of
 * code is executed when leaving the current scope, for example
 * to release a resource or reset a state.
 */
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

bool contains(const StringList &list, const std::string &string);
bool contains(const std::string &string, const std::string &toFind);

std::string listToString(const StringList &list);
std::string inBrackets(const std::string &string);
std::string inSquareBrackets(const std::string &string);
std::string inQuotes(const std::string &string);
std::string boolToString(const bool value);
StringList pathsToStrings(const std::vector<std::filesystem::path> &paths);

bool isPathToFile(const std::string &path);
bool isHeaderFile(const std::string &path);

bool isWhitespace(const char character);
std::string toLower(const std::string &input);

unsigned int terminalWidth();
}; // namespace Tools

#define forever while (true)
