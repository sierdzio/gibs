#include "exported.h"
#include "libraryclass.h"

std::string Exported::text() const
{
    return "another simple one! " + LibraryClass().text();
}
