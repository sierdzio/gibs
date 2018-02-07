//i target name SimpleTestQt
//i qt core

#include "someclass.h"

#include <QDebug>

int main() {
    SomeClass sc;
    qDebug() << sc.text();
    return 0;
}
