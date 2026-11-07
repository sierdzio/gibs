#include "standardpaths.h"

#include <stdexcept>
#include <string_view>

namespace
{
#if defined(__APPLE__)
#define DEFAULT_CONFIGURATION_PATH "~/Library/Application Support/gibs"
#define DEFAULT_COMPILER_SETS_SUFFIX "/compiler-sets"
#elif defined(__unix__)
#define DEFAULT_CONFIGURATION_PATH "~/.config/gibs"
#define DEFAULT_COMPILER_SETS_SUFFIX "/compiler-sets"
#elif defined(_WIN32)
// TODO: Use SHGetKnownFolderPath(FOLDERID_ProgramData) instead of hardcoding the path
#define DEFAULT_CONFIGURATION_PATH "C:\\ProgramData\\gibs"
#define DEFAULT_COMPILER_SETS_SUFFIX "\\compiler-sets"
#else
#define DEFAULT_CONFIGURATION_PATH ""
#define DEFAULT_COMPILER_SETS_SUFFIX ""
#endif

constexpr std::string_view DefaultConfigurationPath = DEFAULT_CONFIGURATION_PATH;
constexpr std::string_view DefaultCompilerSetsPath =
    DEFAULT_CONFIGURATION_PATH DEFAULT_COMPILER_SETS_SUFFIX;

#undef DEFAULT_CONFIGURATION_PATH
#undef DEFAULT_COMPILER_SETS_SUFFIX
} //namespace

std::string_view StandardPaths::configurationPath()
{
    if (DefaultConfigurationPath.empty())
    {
        throw std::runtime_error(
            "Unsupported platform for determining configuration path");
    }

    return DefaultConfigurationPath;
}

StringViewList StandardPaths::compilerSetsPaths()
{
    if (DefaultCompilerSetsPath.empty())
    {
        throw std::runtime_error(
            "Unsupported platform for determining compiler sets path");
    }

    return {DefaultCompilerSetsPath};
}