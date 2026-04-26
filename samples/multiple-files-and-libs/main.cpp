//i executable name MultipleFilesAndLibs

#include "anotherclass.h"
#include "someclass.h"

#include <iostream>

int main()
{
    SomeClass sc;
    std::cout << sc.text() << AnotherClass().text() << std::endl;
    return 0;
}
