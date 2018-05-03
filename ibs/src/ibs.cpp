#include "ibs.h"

void Ibs::removeFile(const QString &path) {
    if (QFile::exists(path)) {
        qInfo() << "Removing:" << path;
        QFile::remove(path);
    }
}
