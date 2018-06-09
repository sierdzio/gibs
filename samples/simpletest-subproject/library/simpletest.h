#pragma once

/*i
 * target name simpletest
 * target type lib dynamic
 * qt core
 */

#include "library_global.h"

class LIBRARYSHARED_EXPORT SimpleTest
{
public:
    const char *text() const;
};
