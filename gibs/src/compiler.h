#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>

struct Compiler
{
    QJsonObject toJson() const;
    static Compiler fromJson(const QJsonObject &json);

    QString name = "gcc";
    QString compiler = "g++";
    QString ccompiler = "gcc";
    QStringList flags = { "-c", "-pipe", "-D_REENTRANT", "-fPIC", "-Wall", "-W", };
    QStringList debugFlags = { "-g" };
    QStringList releaseFlags = { "-O2" };

    QString linker = "g++";
    QString staticArchiver = "ar";
    QString librarySuffix = ".so";
    QString staticLibrarySuffix = ".a";

    QStringList linkerFlags = { "-shared", "-Wl,-soname,lib" };
    QStringList linkerStaticFlags;
    QStringList linkerDynamicFlags;
};
