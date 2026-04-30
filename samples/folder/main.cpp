//i target name SimpleTestFolder
//i includes folder
//    Random text

#include "folder/someclass.h"

#include <iostream>

int main()
{
    SomeClass sc;
    std::cout << sc.text() << std::endl;
    return 0;
}
