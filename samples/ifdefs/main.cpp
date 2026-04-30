//i target name SimpleTestIfdefs
//i qt core

#include "someclass.h"

#ifdef Q_OS_LINUX
#include <iostream>
#endif

int main()
{
#ifdef Q_OS_LINUX
    SomeClass sc;
    std::cout << sc.text() << std::endl;
#else
    // Nothing
#endif
    return 0;
}
