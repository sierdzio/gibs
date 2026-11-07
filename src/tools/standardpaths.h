#include "stringlist.h"
#include <string_view>

class StandardPaths
{
  public:
    static std::string_view configurationPath();
    static StringViewList compilerSetsPaths();
};