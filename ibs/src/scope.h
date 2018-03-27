#pragma once

#include <QByteArray>
#include <QString>
#include <QHash>

#include <QJsonObject>

#include "flags.h"
#include "fileinfo.h"
#include "tags.h"

class Scope
{
public:
    explicit Scope(const QString &name, const QString &relativePath);

    QString name() const;
    QByteArray id() const;

    QJsonObject toJson() const;
    static Scope fromJson(const QJsonObject &json);

    QList<FileInfo> parsedFiles() const;
    void insertParsedFile(const FileInfo &fileInfo);
    FileInfo parsedFile(const QString &path) const;
    bool isParsed(const QString &path) const;

    void addIncludePaths(const QStringList &includes);
    QStringList includePaths() const;
    QStringList customIncludeFlags() const;
    void autoScanForIncludes();    

    void setTargetName(const QString &target);
    void setTargetType(const QString &type);
    void setQtModules(const QStringList &modules);
    void addDefines(const QStringList &defines);
    void addLibs(const QStringList &libs);

    QString findFile(const QString &file) const;

protected:
    Scope();
    QString findFile(const QString &file, const QStringList &includeDirs) const;
    void updateQtModules(const QStringList &modules);
    QString capitalizeFirstLetter(const QString &string) const;

    const QString mRelativePath;
    const QString mName;
    const QByteArray mId;

    QString mQtDir;
    QStringList mQtModules;
    bool mQtIsMocInitialized = false;
    QStringList mQtIncludes;
    QStringList mQtLibs;
    QStringList mQtDefines;

    QStringList mCustomDefines;
    QStringList mCustomDefineFlags;
    //QStringList mCustomIncludes;
    //QStringList mCustomIncludeFlags;
    QStringList mCustomLibs;

    QString mTargetName = "default";
    QString mTargetType = Tags::targetApp;
    QString mTargetLibType = Tags::targetLibDynamic;

    QStringList mCustomIncludes;
    QStringList mCustomIncludeFlags;
    QHash<QString, FileInfo> mParsedFiles;
};
