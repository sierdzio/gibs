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

signals:
    void error(const QString &error) const;
    void finished() const;
    void jobQueueEmpty() const;

public slots:
    void start();
    void clean();
    void runProcess(const QString &app, const QStringList &arguments, const MetaProcessPtr &mp);

protected slots:
    void onError(const QString &error);
    void saveCache() const;
    void onSubproject(const QByteArray &scopeId, const QString &path);

    // Process handling
    void onProcessErrorOccurred(QProcess::ProcessError _error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void runNextProcess();
    void scanForIncludes(const QString &path);

    Flags mFlags;

    bool mIsError = false;
    bool mCacheEnabled = false;

    // TODO: ... what about global settings? Hm.

    // TODO: Allow different Qt dir per subproject (scope)
    QString mQtDir;

    ScopePtr mGlobalScope;

    // scopeId, scope
    QHash<QByteArray, ScopePtr> mScopes;

    QVector<MetaProcessPtr> mProcessQueue;
    QVector<ProcessPtr> mRunningJobs;
};
