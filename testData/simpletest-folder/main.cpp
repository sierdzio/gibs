//i target name SimpleTestFolder
//i includes folder

#include "someclass.h"

#include <iostream>

int main() {
    SomeClass sc;
    std::cout << sc.text() << std::endl;
    return 0;
}
