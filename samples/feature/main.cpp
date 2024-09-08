//i target name SimpleTestFeature
//i qt core
//i feature my-feature someclass.h default off

#ifdef MY_FEATURE
#include "someclass.h"
#endif

#include <QDebug>

int main() {
#ifdef MY_FEATURE
    SomeClass sc;
    qDebug() << sc.text();
#else
    qDebug() << "No feature!";
#endif
    return 0;
}
