#include "stringlist.h"
#include <string_view>

namespace StandardPaths
{
std::string_view configurationPath();
StringViewList compilerSetsPaths();
}; //namespace StandardPaths