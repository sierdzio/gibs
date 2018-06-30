#pragma once

#include <QString>
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
    QString compilerDebugFlags;
    QString compilerReleaseFlags;

    QString linker = "g++";
    QString staticArchiver = "ar";
    QString librarySuffix = ".so";
    QString staticLibrarySuffix = ".a";

    QString linkerDebugFlags;
    QString linkerReleaseFlags;
};
