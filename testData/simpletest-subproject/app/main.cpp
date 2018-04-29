#include "simpletest.h"

#include <iostream>

//i target name SimpleTestSubproject
//i subproject ../library/simpletest.h

int main() {
    SimpleTest sc;
    std::cout << sc.text() << std::endl;
    return 0;
}
