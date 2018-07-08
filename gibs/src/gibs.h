#pragma once

// GIBS code
#include <QString>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <QDebug>

namespace Gibs {
struct Feature {
    QString name;
    QString define;
    bool defined = false;
    bool enabled = false;
};

enum ToolType {
    Compiler,
    Deployer
};

void removeFile(const QString &path);
QString findFile(const QString &directory, const QString &name);
QString findJsonToolDefinition(const QString &tool, const ToolType type);
QJsonDocument readJsonFile(const QString &path);

QString normalizeFeatureName(const QString &name);
Feature commandLineToFeature(const QString &command);
QString capitalizeFirstLetter(const QString &string);

QStringList jsonArrayToStringList(const QJsonArray &array);

QString ifEmpty(const QString &value, const QString &defaultValue);
}
