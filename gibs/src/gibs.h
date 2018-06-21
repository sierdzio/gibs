#pragma once

// GIBS code
#include <QString>
#include <QFile>
#include <QDebug>

namespace Gibs {
void removeFile(const QString &path);
QString normalizeFeatureName(const QString &name);
}
