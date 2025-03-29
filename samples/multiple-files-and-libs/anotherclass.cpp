
#include "anotherclass.h"

//! include library lib
#include <exported.h>

std::string AnotherClass::text() const
{
    std::string result("another simple one!");
    result.append(" From library: ");
    result.append(Exported().text());

    return result;
}
