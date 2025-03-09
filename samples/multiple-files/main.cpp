//i target name MultipleFiles

#include "someclass.h"
#include "anotherclass.h"

#include <stdio>

int main() {
    SomeClass sc;
    std::cout << sc.text() << AnotherClass().text() << std::endl;
    return 0;
}
