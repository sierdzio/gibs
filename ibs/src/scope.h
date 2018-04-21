#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QHash>

#include <QJsonObject>
#include <QJsonArray>

#include "flags.h"
#include "fileinfo.h"
#include "tags.h"

class Scope : public QObject
{
    Q_OBJECT

public:
    explicit Scope(const QString &name, const QString &relativePath,
                   QObject *parent = nullptr);

    QString name() const;
    QByteArray id() const;

    QJsonObject toJson() const;
    static Scope *fromJson(const QJsonObject &json);

    QList<FileInfo> parsedFiles() const;
    void insertParsedFile(const FileInfo &fileInfo);
    FileInfo parsedFile(const QString &path) const;
    bool isParsed(const QString &path) const;

    QStringList includePaths() const;
    QStringList customIncludeFlags() const;
    void autoScanForIncludes();    

    QString findFile(const QString &file) const;

    QStringList qtModules() const;

    QStringList customDefineFlags() const;

    QStringList qtDefines() const;

    QStringList qtIncludes() const;

    QStringList qtLibs() const;

    QString targetName() const;

    QString targetType() const;

    QString targetLibType() const;

    QStringList customLibs() const;

    bool qtIsMocInitialized() const;
    void setQtIsMocInitialized(bool qtIsMocInitialized);

public slots:
    void addIncludePaths(const QStringList &includes);
    void setTargetName(const QString &target);
    void setTargetType(const QString &type);
    void setQtModules(const QStringList &modules);
    void addDefines(const QStringList &defines);
    void addLibs(const QStringList &libs);


protected:
    Scope();
    QString findFile(const QString &file, const QStringList &includeDirs) const;
    void updateQtModules(const QStringList &modules);
    QString capitalizeFirstLetter(const QString &string) const;
    QStringList jsonArrayToStringList(const QJsonArray &array) const;

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
