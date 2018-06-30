#pragma once

// GIBS code
#include <QString>
#include <QFile>
#include <QDebug>

namespace Gibs {
struct Feature {
    QString name;
    QString define;
    bool defined = false;
    bool enabled = false;
};

void removeFile(const QString &path);
QString normalizeFeatureName(const QString &name);
Feature commandLineToFeature(const QString &command);
QString capitalizeFirstLetter(const QString &string);
QStringList jsonArrayToStringList(const QJsonArray &array);
}
