//i target name SimpleTestFeature
//i qt core
//i feature my-feature someclass.h default off

#ifdef MY_FEATURE
#include "someclass.h"
#endif

#include <iostream>

int main()
{
#ifdef MY_FEATURE
    SomeClass sc;
    std::cout << sc.text() << std::endl;
#else
    std::cout << "No feature!" << std::endl;
#endif
    return 0;
}
