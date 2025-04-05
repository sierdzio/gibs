// i target name SimpleTestSubproject
// i subproject ../library/simpletest.h

#include "simpletest.h"

#include <iostream>

int main()
{
    SimpleTest sc;
    std::cout << sc.text() << std::endl;
    return 0;
}
