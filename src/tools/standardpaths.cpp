#include "standardpaths.h"

#include <stdexcept>

StringViewList StandardPaths::compilerSetsPaths()
{
#ifdef __unix__
    return {"~/.config/gibs/compiler-sets"};
#elif defined(__APPLE__)
    return {"~/Library/Application Support/gibs/compiler-sets"};
#elif defined(_WIN32)
    // TODO: Use SHGetKnownFolderPath(FOLDERID_ProgramData) instead of hardcoding the path
    return {"C:\\ProgramData\\gibs\\compiler-sets"};
#else
    throw std::runtime_error("Unsupported platform for determining compiler sets path");
#endif
}