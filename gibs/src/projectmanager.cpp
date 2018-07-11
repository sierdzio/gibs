#include "projectmanager.h"
#include "gibs.h"
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
#include <QTimer>

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
    if (qtDir == mFlags.qtDir) {
        return;
    }

    qInfo() << "Setting Qt directory to:" << qtDir;
    mFlags.qtDir = qtDir;

    // TODO: upgrade mQtLibs and mQtIncludes
}

QString ProjectManager::qtDir() const
{
    return mFlags.qtDir;
}

void ProjectManager::start()
{
    QByteArray tempScopeId;

    // First, check if any files need to be recompiled
    if (mCacheEnabled) {
        const auto scopes = mScopes.values();
        for (const auto &scope : scopes) {
            scope->start(true, mFlags.quickMode);
        }
    } else {
        ScopePtr scope = ScopePtr::create(mFlags.inputFile,
                                          mFlags.relativePath(),
                                          mFlags,
                                          mFeatures);
        connectScope(scope);

        if (mFlags.compilerName != "gcc") {
            const QString compiler(Compiler::find(mFlags.compilerName));
            if (compiler.isEmpty()) {
                qFatal("Could not find compiler named: %s in $HOME/.gibs, nor in "
                       "internal database", qPrintable(compiler));
            } else {
                scope->setCompiler(Compiler::fromFile(compiler));
            }
        }

        if (!mFlags.deployerName.isEmpty()) {
            const QString deployer(Deployer::find(mFlags.deployerName));
            if (deployer.isEmpty()) {
                qFatal("Could not find deployer named: %s in $HOME/.gibs, nor in "
                       "internal database", qPrintable(deployer));
            } else {
                scope->setDeployer(Deployer::fromFile(deployer));
            }
        }

        if (!mGlobalScope.isNull()) {
            scope->mergeWith(mGlobalScope);
        }

        mScopes.insert(scope->id(), scope);

        if (mFlags.autoIncludes)
            scope->autoScanForIncludes();

        scope->start(false, mFlags.quickMode);
        tempScopeId = scope->id();
    }

    saveCache();
}

void ProjectManager::clean()
{
    const auto scopes = mScopes.values();
    for (const auto &scope : scopes) {
        scope->clean();
    }

    emit finished(0);
}

void ProjectManager::onError(const QString &error)
{
    /*qCritical()*/ qFatal("Error! %s", qPrintable(error));
    //mIsError = true;
}

/*!
 * Save necessary build info into GIBS cache file
 */
void ProjectManager::saveCache() const
{
    QFile file(Tags::gibsCacheFileName);

    if (file.exists())
        file.remove();

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qFatal("Could not open GIBS cache file for writing!");
    }

    QJsonObject mainObject;

    mainObject.insert(Tags::qtDir, mFlags.qtDir);
    mainObject.insert(Tags::inputFile, mFlags.inputFile);

    QJsonArray scopesArray;
    const auto scopes = mScopes.values();
    for (const auto &scope : scopes) {
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
 * Load necessary build info from GIBS cache file
 */
void ProjectManager::loadCache()
{
    QFile file(Tags::gibsCacheFileName);

    if (!file.exists()) {
        qInfo("Cannot find GIBS cache file");
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qFatal("Could not open GIBS cache file for reading!");
    }

    qInfo() << "Loading gibs cache file" << Tags::gibsCacheFileName;

    //const auto document = QJsonDocument::fromBinaryData(file.readAll());
    const auto document = QJsonDocument::fromJson(file.readAll());
    const QJsonObject mainObject(document.object());

    mFlags.qtDir = mainObject.value(Tags::qtDir).toString();

    const QJsonArray scopesArray = mainObject.value(Tags::scopes).toArray();
    for (const auto &scopeJson : scopesArray) {
        ScopePtr scope(Scope::fromJson(scopeJson.toObject(), mFlags));
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
        const auto scopeIds = scope->scopeDependencyIds();
        for (const auto &scopeId : scopeIds) {
            scope->dependOn(mScopes.value(scopeId));
        }
    }

    if (mFlags.inputFile.isEmpty()) {
        mFlags.inputFile = mainObject.value(Tags::inputFile).toString();
    } else {
        // TODO: if input file was specified and is diferent than the one in cache
        // we need to invalidate the cache!
    }


    mCacheEnabled = true;
}

void ProjectManager::loadCommands()
{
    if (mFlags.commands.isEmpty())
        return;

    qInfo() << "Loading commands from command line";

    if (mGlobalScope.isNull()) {
        mGlobalScope = ScopePtr::create(Tags::globalScope, mFlags.relativePath(),
                                        mFlags,
                                        mFeatures);
        connectScope(mGlobalScope);
        mScopes.insert(mGlobalScope->id(), mGlobalScope);
    }

    CommandParser parser(mFlags.commands, mGlobalScope.data());
    connect(&parser, &CommandParser::error, this, &ProjectManager::error);
    connect(&parser, &CommandParser::subproject, this, &ProjectManager::onSubproject);
    parser.parse();
}

void ProjectManager::loadFeatures(const QHash<QString, Gibs::Feature> &features)
{
    mFeatures = features;
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
    //qDebug() << "Subproject" << path << "AAA" << oldScope->relativePath() + "/" + path;
    const QString newRelativePath(QFileInfo(path).path());
    ScopePtr scope = ScopePtr::create(oldScope->relativePath() + "/" + path,
                                      oldScope->relativePath() + "/" + newRelativePath,
                                      mFlags,
                                      mFeatures);
    //qDebug() << "Subproject:" << scope->name() << "STARTING!";
    connectScope(scope);

    // TODO: as subproject is being parsed, it should also set includepath and
    // LIBS in the depending scope

    oldScope->dependOn(scope);
    if (!mGlobalScope.isNull()) {
        scope->mergeWith(mGlobalScope);
    }
    mScopes.insert(scope->id(), scope);
    scope->start(false, mFlags.quickMode);
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

void ProjectManager::onFeatureUpdated(const Gibs::Feature &feature)
{
    mFeatures.insert(feature.name, feature);
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
            const auto mp = mProcessQueue.at(i);
            if (mp->hasFinished or !mp->canRun() or mp->process->state() == QProcess::Running
                    or mp->process->state() == QProcess::Starting) {
                continue;
            }

            if (mp->scopeDepenencies.isEmpty() == false) {
                const QString blockee(nextBlockingScopeName(mp));
                if (!blockee.isEmpty()) {
                    qDebug() << "Waiting for scope:" << blockee;
                    //QCoreApplication::instance()->processEvents();
                    //continue;
                    QTimer::singleShot(16, this, &ProjectManager::runNextProcess);
                }
            }

            mRunningJobs.append(mp->process);
            qInfo() << "Running next process:" << i << mRunningJobs.last()->program() << mRunningJobs.last()->arguments().join(" ");
            mRunningJobs.last()->start();
        }

        // Start working asap
        QCoreApplication::instance()->processEvents();
    }

    if (mProcessQueue.isEmpty()) {
        emit jobQueueEmpty(mIsError);
    }
}

QString ProjectManager::nextBlockingScopeName(const MetaProcessPtr &mp) const
{
    for (const auto &scopeId : qAsConst(mp->scopeDepenencies)) {
        if (mScopes.value(scopeId)->isFinished() == false) {
            return mScopes.value(scopeId)->name();
        }
    }

    return QString();
}

void ProjectManager::connectScope(const ScopePtr &scope)
{
    connect(scope.data(), &Scope::error,
            this, &ProjectManager::error);
    connect(scope.data(), &Scope::subproject,
            this, &ProjectManager::onSubproject);
    connect(scope.data(), &Scope::runProcess,
            this, &ProjectManager::runProcess,
            Qt::QueuedConnection);
    connect(scope.data(), &Scope::feature,
            this, &ProjectManager::onFeatureUpdated);
}
