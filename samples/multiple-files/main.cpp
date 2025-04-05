//i executable name MultipleFiles

#include "anotherclass.h"
#include "someclass.h"

#include <stdio>

int main()
{
    SomeClass sc;
    std::cout << sc.text() << AnotherClass().text() << std::endl;
    return 0;
}
