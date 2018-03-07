#pragma once

#include <QObject>
#include <QHash>
#include <QDateTime>

#include "tags.h"
#include "flags.h"
#include "fileinfo.h"

class QJsonArray;

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(const Flags &flags, QObject *parent = nullptr);
    virtual ~ProjectManager();

    void setQtDir(const QString &qtDir);
    QString qtDir() const;

    void loadCache();

signals:
    void error(const QString &error) const;
    void finished() const;

public slots:
    void start();
    void clean();

protected slots:
    void onError(const QString &error);
    void saveCache() const;
    bool isFileDirty(const QString &file, const bool isQuickMode) const;

    void onParsed(const QString &file, const QString &source,
                  const QByteArray &checksum,
                  const QDateTime &modified,
                  const QDateTime &created);
    void onParseRequest(const QString &file, const bool force = false);
    bool onRunMoc(const QString &file);
    // ibs commands
    void onTargetName(const QString &target);
    void onTargetType(const QString &type);
    void onQtModules(const QStringList &modules);
    void onDefines(const QStringList &defines);
    void onIncludes(const QStringList &includes);
    void onLibs(const QStringList &libs);
    void onRunTool(const QString &tool, const QStringList &args);

protected:
    QString compile(const QString &file);
    void link() const;
    void parseFile(const QString &file);

private:
    void updateQtModules(const QStringList &modules);
    bool initializeMoc();
    bool runProcess(const QString &app, const QStringList &arguments) const;
    QString capitalizeFirstLetter(const QString &string) const;
    QString findFile(const QString &file, const QStringList &includeDirs) const;
    QStringList jsonArrayToStringList(const QJsonArray &array) const;
    void removeFile(const QString &path) const;

    const Flags mFlags;

    bool mIsError = false;
    bool mCacheEnabled = false;

    QString mQtDir;
    QStringList mQtModules;
    bool mQtIsMocInitialized = false;
    QStringList mQtIncludes;
    QStringList mQtLibs;
    QStringList mQtDefines;

    QStringList mCustomDefines;
    QStringList mCustomDefineFlags;
    QStringList mCustomIncludes;
    QStringList mCustomIncludeFlags;
    QStringList mCustomLibs;

    QString mTargetName = "default";
    QString mTargetType = Tags::targetApp;
    QString mTargetLibType = Tags::targetLibDynamic;
    QHash<QString, FileInfo> mParsedFiles;
};
