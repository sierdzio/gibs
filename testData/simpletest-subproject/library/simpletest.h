#pragma once

#include "library_global.h"

/*i
 * target name simpletest
 * target type lib dynamic
 * qt core
 */

class LIBRARYSHARED_EXPORT SimpleTest
{
public:
    const char *text() const;
};
