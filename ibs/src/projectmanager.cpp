#include "projectmanager.h"
#include "ibs.h"
#include "fileparser.h"
#include "commandparser.h"

#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QCoreApplication>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>

// TODO: add categorized logging!
#include <QDebug>

ProjectManager::ProjectManager(const Flags &flags, QObject *parent)
    : QObject(parent), mFlags(flags)
{
    qRegisterMetaType<MetaProcess>("MetaProcess");
    qRegisterMetaType<MetaProcessPtr>("MetaProcessPtr");

    // When all jobs are done, notify main.cpp that we can quit
    connect(this, &ProjectManager::jobQueueEmpty, this, &ProjectManager::finished);

    connect(this, &ProjectManager::error, this, &ProjectManager::onError);
}

ProjectManager::~ProjectManager()
{
}

void ProjectManager::setQtDir(const QString &qtDir)
{
    if (qtDir == mFlags.qtDir()) {
        return;
    }

    qInfo() << "Setting Qt directory to:" << qtDir;
    mFlags.setQtDir(qtDir);

    // TODO: upgrade mQtLibs and mQtIncludes
}

QString ProjectManager::qtDir() const
{
    return mFlags.qtDir();
}

void ProjectManager::start()
{
    QByteArray tempScopeId;

    // First, check if any files need to be recompiled
    if (mCacheEnabled) {
        for (const auto &scope : mScopes.values()) {
            scope->start(true, mFlags.quickMode());
        }
    } else {
        ScopePtr scope = ScopePtr::create(mFlags.inputFile(), mFlags.relativePath(),
                                          mFlags.prefix(), mFlags.qtDir());
        connectScope(scope);

        if (!mGlobalScope.isNull()) {
            scope->mergeWith(mGlobalScope);
        }

        mScopes.insert(scope->id(), scope);

        if (mFlags.autoIncludes())
            scope->autoScanForIncludes();

        scope->start(false, mFlags.quickMode());
        tempScopeId = scope->id();
    }

    saveCache();
}

void ProjectManager::clean()
{
    for (const auto &scope : mScopes.values()) {
        scope->clean();
    }

    emit finished(0);
}

void ProjectManager::onError(const QString &error)
{
    qCritical() << "Error!" << error;
    mIsError = true;
}

/*!
 * Save necessary build info into IBS cache file
 */
void ProjectManager::saveCache() const
{
    QFile file(Tags::ibsCacheFileName);

    if (file.exists())
        file.remove();

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qFatal("Could not open IBS cache file for writing!");
    }

    QJsonObject mainObject;

    mainObject.insert(Tags::qtDir, mFlags.qtDir());
    mainObject.insert(Tags::inputFile, mFlags.inputFile());

    QJsonArray scopesArray;
    for (const auto &scope : mScopes.values()) {
        scopesArray.append(scope->toJson());
    }
    mainObject.insert(Tags::scopes, scopesArray);

    // TODO: save also all tools that need to be run!

    const QJsonDocument document(mainObject);
    // TODO: switch back to binary format after testing
    //const auto data = document.toBinaryData();
    const auto data = document.toJson();
    const auto result = file.write(data);

    if (data.size() != result) {
        qWarning() << "Not all cache data has been saved." << data.size()
                   << "vs" << result;
    }
}

/*!
 * Load necessary build info from IBS cache file
 */
void ProjectManager::loadCache()
{
    QFile file(Tags::ibsCacheFileName);

    if (!file.exists()) {
        qInfo("Cannot find IBS cache file");
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qFatal("Could not open IBS cache file for reading!");
    }

    qInfo() << "Loading ibs cache file" << Tags::ibsCacheFileName;

    //const auto document = QJsonDocument::fromBinaryData(file.readAll());
    const auto document = QJsonDocument::fromJson(file.readAll());
    const QJsonObject mainObject(document.object());

    mFlags.setQtDir(mainObject.value(Tags::qtDir).toString());

    const QJsonArray scopesArray = mainObject.value(Tags::scopes).toArray();
    for (const auto &scopeJson : scopesArray) {
        ScopePtr scope(Scope::fromJson(scopeJson.toObject()));
        mScopes.insert(scope->id(), scope);
        if (scope->name() == Tags::globalScope) {
            mGlobalScope = scope;
        }

        connect(scope.data(), &Scope::error, this, &ProjectManager::error);
        connect(scope.data(), &Scope::subproject, this, &ProjectManager::onSubproject);
        connect(scope.data(), &Scope::runProcess, this, &ProjectManager::runProcess,
                Qt::QueuedConnection);
    }

    for (const auto &scope : qAsConst(mScopes)) {
        for (const auto &scopeId : scope->scopeDependencyIds()) {
            scope->dependOn(mScopes.value(scopeId));
        }
    }

    if (mFlags.inputFile().isEmpty()) {
        mFlags.setInputFile(mainObject.value(Tags::inputFile).toString());
    } else {
        // TODO: if input file was specified and is diferent than the one in cache
        // we need to invalidate the cache!
    }


    mCacheEnabled = true;
}

void ProjectManager::loadCommands()
{
    if (mFlags.commands().isEmpty())
        return;

    qInfo() << "Loading commands from command line";

    if (mGlobalScope.isNull()) {
        mGlobalScope = ScopePtr::create(Tags::globalScope, mFlags.relativePath(),
                                        mFlags.prefix(), mFlags.qtDir());
        connectScope(mGlobalScope);
        mScopes.insert(mGlobalScope->id(), mGlobalScope);
    }

    CommandParser parser(mFlags.commands(), mGlobalScope.data());
    connect(&parser, &CommandParser::error, this, &ProjectManager::error);
    connect(&parser, &CommandParser::subproject, this, &ProjectManager::onSubproject);
    parser.parse();
}

/*!
 * Register new subproject. If current scope (\a scopeId) depends on the new
 * project, it will be added to \a scopeId later.
 *
 * \a path contains the main file of the subproject.
 */
void ProjectManager::onSubproject(const QByteArray &scopeId, const QString &path)
{
    auto oldScope = mScopes.value(scopeId);
    const QString newRelativePath(QFileInfo(path).path());
    ScopePtr scope = ScopePtr::create(oldScope->relativePath() + "/" + path,
                                      newRelativePath,
                                      mFlags.prefix(),
                                      mFlags.qtDir());
    //qDebug() << "Subproject:" << scope->name() << "STARTING!";
    connectScope(scope);

    // TODO: as subproject is being parsed, it should also set includepath and
    // LIBS in the depending scope

    oldScope->dependOn(scope);
    if (!mGlobalScope.isNull()) {
        scope->mergeWith(mGlobalScope);
    }
    mScopes.insert(scope->id(), scope);
    scope->start(false, mFlags.quickMode());
    //qDebug() << "Subproject:" << scope->name() << "FINISHED!";
    // TODO: this has to be made conditional: only when subproject is actually
    // a library (and not an app, or type zero, or plugin).
    // Update INCLUDEPATH
    oldScope->addIncludePaths(scope->includePaths());
    oldScope->addIncludePaths(QStringList {scope->relativePath()});
    // Update LIBS
    oldScope->addLibs(QStringList {
                          //"-L" + oldScope->relativePath() + "/" + newRelativePath,
                          "-L" + mFlags.prefix(),
                          "-l" + scope->targetName()
                      });
}

void ProjectManager::onProcessErrorOccurred(QProcess::ProcessError _error)
{
    auto process = qobject_cast<QProcess *>(sender());

    if (process == nullptr) {
        emit error("Process finished, but could not be accessed. :-(");
        return;
    }

    // Remove process from the queue
    for (int i = 0; i < mProcessQueue.count(); ++i) {
        if (process == mProcessQueue.at(i)->process) {
            mProcessQueue.remove(i);
            break;
        }
    }

//    for (int i = 0; i < mRunningJobs.count(); ++i) {
//        if (process == mRunningJobs.at(i)) {
//            // Process will be auto-deleted by QSharedPointer
//            mRunningJobs.remove(i);
//            break;
//        }
//    }

    emit error(QString("Process %1: error occurred: %2")
               .arg(process->program(),
                    QString(_error))
               );
}

void ProjectManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    auto process = qobject_cast<QProcess *>(sender());

    if (process == nullptr) {
        emit error("Process finished, but could not be accessed. :-(");
        return;
    }

    if (exitCode != 0) {
        emit error(QString("Process %1: finished with exit code %2 and status %3")
                   .arg(process->program(),
                        QString::number(exitCode),
                        QString::number(exitStatus)));
    }

    // Remove process from the queue
    for (int i = 0; i < mProcessQueue.count(); ++i) {
        if (process == mProcessQueue.at(i)->process) {
            mProcessQueue.at(i)->hasFinished = true;
            mProcessQueue.remove(i);
            break;
        }
    }

    for (int i = 0; i < mRunningJobs.count(); ++i) {
        if (process == mRunningJobs.at(i)) {
            // Process will be auto-deleted by QSharedPointer
            mRunningJobs.remove(i);
            break;
        }
    }

    runNextProcess();
}

void ProjectManager::runProcess(const QString &app, const QStringList &arguments,
                                const MetaProcessPtr &mp)
{
    auto process = new QProcess();

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProjectManager::onProcessFinished);

    connect(process, &QProcess::errorOccurred,
            this, &ProjectManager::onProcessErrorOccurred);

    process->setProcessChannelMode(QProcess::ForwardedChannels);
    process->setProgram(app);
    process->setArguments(arguments);

    mp->process.reset(process);
    mProcessQueue.append(mp);
    runNextProcess();
}

void ProjectManager::runNextProcess()
{
    if (mIsError) {
        // qDebug() << "Not running next process because of an error!";
        emit jobQueueEmpty(true);
        return;
    }

    // TODO: use these counts to create a progress bar as in cmake!
    //qDebug() << "Running jobs:" << mRunningJobs.count() << "max jobs:" << mFlags.jobs << "process queue" << mProcessQueue.count();

    // Run next process if max number of jobs is not exceeded
    if ((mProcessQueue.count() > 0) and (mRunningJobs.count() < mFlags.jobs())) {
        for (int i = 0; i < mProcessQueue.count() and (mRunningJobs.count() < mFlags.jobs()); ++i) {
            const auto & mp = mProcessQueue.at(i);
            if (mp->hasFinished or !mp->canRun() or mp->process->state() == QProcess::Running
                    or mp->process->state() == QProcess::Starting) {
                continue;
            }

            if (mp->scopeDepenencies.isEmpty() == false) {
                for (const auto &scopeId : qAsConst(mp->scopeDepenencies)) {
                    if (mScopes.value(scopeId)->isFinished() == false) {
                        qDebug() << "Waiting for scope:" << mScopes.value(scopeId)->name();
                        continue;
                    }
                }
            }

            mRunningJobs.append(mp->process);
            qInfo() << "Running next process:" << i << mRunningJobs.last()->program() << mRunningJobs.last()->arguments().join(" ");
            mRunningJobs.last()->start();
        }

        // Start working asap
        QCoreApplication::instance()->processEvents();
    }

    if (mProcessQueue.isEmpty())
        emit jobQueueEmpty(mIsError);
}

void ProjectManager::connectScope(const ScopePtr &scope)
{
    connect(scope.data(), &Scope::error, this, &ProjectManager::error);
    connect(scope.data(), &Scope::subproject, this, &ProjectManager::onSubproject);
    connect(scope.data(), &Scope::runProcess, this, &ProjectManager::runProcess,
            Qt::QueuedConnection);
}
