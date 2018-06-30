#pragma once

#include <QObject>
#include <QHash>
#include <QDateTime>
#include <QPointer>

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
    void loadFeatures(const QHash<QString, Gibs::Feature> &features);

signals:
    void error(const QString &error) const;
    void finished(const int returnValue) const;
    void jobQueueEmpty(const bool isError) const;

public slots:
    void start();
    void clean();
    void runProcess(const QString &app, const QStringList &arguments, const MetaProcessPtr &mp);

protected slots:
    void onError(const QString &error);
    void saveCache() const;
    void onSubproject(const QByteArray &scopeId, const QString &path);
    void onFeatureUpdated(const Gibs::Feature &feature);

    // Process handling
    void onProcessErrorOccurred(QProcess::ProcessError _error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void runNextProcess();
    QString nextBlockingScopeName(const MetaProcessPtr &mp) const;
    void scanForIncludes(const QString &path);
    void connectScope(const ScopePtr &scope);

    bool mIsError = false;
    bool mCacheEnabled = false;

    Flags mFlags;
    ScopePtr mGlobalScope;

    // scopeId, scope
    QHash<QByteArray, ScopePtr> mScopes;
    // name, Feature
    QHash<QString, Gibs::Feature> mFeatures;

    QVector<MetaProcessPtr> mProcessQueue;
    QVector<ProcessPtr> mRunningJobs;
};
