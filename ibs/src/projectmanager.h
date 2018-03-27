#pragma once

#include <QObject>
#include <QHash>
#include <QDateTime>

// Process handling
#include <QProcess>
#include <QVector>

#include "tags.h"
#include "flags.h"
#include "scope.h"
#include "fileinfo.h"
#include "metaprocess.h"

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
    void loadCommands();

signals:
    void error(const QString &error) const;
    void finished() const;
    void jobQueueEmpty() const;

public slots:
    void start();
    void clean();

protected slots:
    void onError(const QString &error);
    void saveCache() const;
    bool isFileDirty(const QString &file, const bool isQuickMode,
                     const Scope &scope) const;

    void onParsed(const QByteArray &scopeId,
                  const QString &file, const QString &source,
                  const QByteArray &checksum,
                  const QDateTime &modified,
                  const QDateTime &created);
    void onParseRequest(const QByteArray &scopeId, const QString &file,
                        const bool force = false);
    bool onRunMoc(const QByteArray &scopeId, const QString &file);
    // ibs commands
//    void onTargetName(const QString &target);
//    void onTargetType(const QString &type);
//    void onQtModules(const QStringList &modules);
//    void onDefines(const QStringList &defines);
//    void onIncludes(const QByteArray &scopeId, const QStringList &includes);
//    void onLibs(const QStringList &libs);
    void onRunTool(const QByteArray &scopeId, const QString &tool,
                   const QStringList &args);

    // Process handling
    void onProcessErrorOccurred(QProcess::ProcessError _error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    QString compile(const QByteArray &scopeId, const QString &file);
    void link(const QByteArray &scopeId);
    void parseFile(const QByteArray &scopeId, const QString &file);

private:
    //void updateQtModules(const QStringList &modules);
    bool initializeMoc(const QByteArray &scopeId);
    void runProcess(const QString &app, const QStringList &arguments, MetaProcess mp);
    void runNextProcess();
    ProcessPtr findDependency(const QString &file) const;
    QVector<ProcessPtr> findDependencies(const QString &file) const;
    QVector<ProcessPtr> findAllDependencies() const;
//    QString capitalizeFirstLetter(const QString &string) const;
    //QString findFile(const QString &file, const QStringList &includeDirs) const;
    QStringList jsonArrayToStringList(const QJsonArray &array) const;
    void removeFile(const QString &path) const;
    void scanForIncludes(const QString &path);

    Flags mFlags;

    bool mIsError = false;
    bool mCacheEnabled = false;

    // TODO: ... what about global settings? Hm.

//    QString mQtDir;
//    QStringList mQtModules;
//    bool mQtIsMocInitialized = false;
//    QStringList mQtIncludes;
//    QStringList mQtLibs;
//    QStringList mQtDefines;

//    QStringList mCustomDefines;
//    QStringList mCustomDefineFlags;
//    //QStringList mCustomIncludes;
//    //QStringList mCustomIncludeFlags;
//    QStringList mCustomLibs;

//    QString mTargetName = "default";
//    QString mTargetType = Tags::targetApp;
//    QString mTargetLibType = Tags::targetLibDynamic;
    //QHash<QString, FileInfo> mParsedFiles;
    // scopeId, scope
    QHash<QByteArray, Scope> mScopes;

    QVector<MetaProcess> mProcessQueue;
    QVector<QProcess *> mRunningJobs;
};
