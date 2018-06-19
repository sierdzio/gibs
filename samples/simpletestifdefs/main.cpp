//i target name SimpleTestIfdefs
//i qt core

#include "someclass.h"

#ifdef Q_OS_LINUX
#include <QDebug>
#endif

int main() {
    SomeClass sc;
#ifdef Q_OS_LINUX
    qDebug() << sc.text();
#else
    // Nothing
#endif
    return 0;
}
