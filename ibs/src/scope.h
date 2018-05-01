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

    void mergeWith(Scope *other);
    void dependOn(Scope *other);

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
    QString qtDir() const;

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
    QString compile(const QString &file);
    void link();
    void parseFile(const QString &file);

protected slots:
    void onParsed(const QString &file, const QString &source,
                  const QByteArray &checksum,
                  const QDateTime &modified,
                  const QDateTime &created);
    void onParseRequest(const QString &file,
                        const bool force = false);
    bool onRunMoc(const QString &file);
    void onRunTool(const QString &tool,
                   const QStringList &args);
    //void onSubproject(const QByteArray &scopeId, const QString &path);


protected:
    Scope();
    QString findFile(const QString &file, const QStringList &includeDirs) const;
    void updateQtModules(const QStringList &modules);
    QString capitalizeFirstLetter(const QString &string) const;
    QStringList jsonArrayToStringList(const QJsonArray &array) const;

    const QString mRelativePath;
    const QString mName;
    const QByteArray mId;

    bool mIsError = false;

    QString mQtDir;
    QStringList mQtModules;
    bool mQtIsMocInitialized = false;
    QStringList mQtIncludes;
    QStringList mQtLibs;
    QStringList mQtDefines;

    QStringList mCustomDefines;
    QStringList mCustomDefineFlags;
    QStringList mCustomLibs;

    QString mTargetName = "default";
    QString mTargetType = Tags::targetApp;
    QString mTargetLibType = Tags::targetLibDynamic;

    QStringList mCustomIncludes;
    QStringList mCustomIncludeFlags;
    QHash<QString, FileInfo> mParsedFiles;
};
