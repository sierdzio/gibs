#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>

class Compiler
{
public:
    Compiler();

    QJsonObject toJson() const;
    static Compiler fromJson(const QJsonObject &json);

    QString name = "gcc";
    QString compiler = "g++";
    QString ccompiler = "gcc";
    QStringList compilerFlags = { "-c", "-pipe", "-D_REENTRANT", "-fPIC", "-Wall", "-W", };
    QStringList compilerDebugFlags = { "-g" };
    QStringList compilerReleaseFlags = { "-O2" };

    QString linker = "g++";
    QString staticArchiver = "ar";
    QString librarySuffix = ".so";
    QString staticLibrarySuffix = ".a";

    QStringList linkerFlags;
    QStringList linkerDebugFlags;
    QStringList linkerReleaseFlags;
};
