#ifdef Q_OS_WIN
#include "windows.h"
#endif

#include "someclass.h"

QString SomeClass::text() const
{
    return "simple!";
}
