#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>

/*!
 * \brief The Compiler struct - contains all necessary info to run compilation
 * using given compiler.
 *
 * \todo Use similar construct for DeploymentTool
 *
 * \todo Add more arguments here, use %1 notation from QString to fill them in
 * dynamically at runtime (with target name for example). Something like:
 * QString linkerOutputFile = "lib%1.so";
 */
struct Compiler
{
    QJsonObject toJson() const;
    static Compiler fromJson(const QJsonObject &json);
    static Compiler fromFile(const QString &jsonFile);
    static QString find(const QString &name);

    QString name = "gcc";
    QString compiler = "g++";
    QString ccompiler = "gcc";
    QStringList flags = { "-c", "-pipe", "-D_REENTRANT", "-fPIC", "-Wall", "-W", };
    QStringList debugFlags = { "-g" };
    QStringList releaseFlags = { "-O2" };

    QString linker = "g++";
    QString staticArchiver = "ar";
    QString libraryPrefix = "lib";
    QString librarySuffix = ".so";
    QString staticLibrarySuffix = ".a";

    QStringList linkerFlags = { "-shared", "-Wl,-soname,lib" };
    QStringList linkerStaticFlags;
    QStringList linkerDynamicFlags;
};
