#include "projectmanager.h"
#include "fileparser.h"

#include <QProcess>
#include <QFileInfo>

// TODO: add categorized logging!
#include <QDebug>

ProjectManager::ProjectManager(const QString &inputFile, QObject *parent)
    : QObject(parent), mInputFile(inputFile)
{
}

void ProjectManager::setQtDir(const QString &qtDir)
{
    qInfo() << "Setting Qt directory to:" << qtDir;
    mQtDir = qtDir;
}

void ProjectManager::start()
{
    onParseRequest(mInputFile);
    // Parsing done, link it!
    link();

    emit finished();
}

void ProjectManager::onParsed(const QString &file, const QString &source)
{
    if (!source.isEmpty() and source == file) {
        compile(source);
    }
}

void ProjectManager::onParseRequest(const QString &file)
{
    // Skip files which we have parsed already
    if (mParsedFiles.contains(file))
        return;

    // Prevent file from being parsed twice
    mParsedFiles.append(file);

    FileParser parser(file);
    connect(&parser, &FileParser::parsed, this, &ProjectManager::onParsed);
    connect(&parser, &FileParser::parseRequest, this, &ProjectManager::onParseRequest);
    connect(&parser, &FileParser::runMoc, this, &ProjectManager::onRunMoc);
    connect(&parser, &FileParser::targetName, this, &ProjectManager::onTargetName);
    connect(&parser, &FileParser::targetType, this, &ProjectManager::onTargetType);
    connect(&parser, &FileParser::qtModules, this, &ProjectManager::onQtModules);
    connect(&parser, &FileParser::includes, this, &ProjectManager::onIncludes);
    connect(&parser, &FileParser::libs, this, &ProjectManager::onLibs);

    const bool result = parser.parse();

    // TODO: stop if result is false
    Q_UNUSED(result);
}

bool ProjectManager::onRunMoc(const QString &file)
{
    if (mQtDir.isEmpty()) {
        qFatal("Can't run MOC because Qt dir is not set. See 'ibs --help' for "
               "more info.");
    }

    if (mQtIsMocInitialized == false) {
        if (initializeMoc() == false)
            return false;
    }

    const QFileInfo header(file);
    const QString mocFile("moc_" + header.baseName() + ".cpp");
    const QString compiler(mQtDir + "/bin/moc");

    QStringList arguments;
    arguments.append(mQtDefines);
    arguments.append({ "--include", "moc_predefs.h" });
    arguments.append(mQtIncludes);
    // TODO: GCC includes!
    arguments.append({ file, "-o", mocFile });

    if (runProcess(compiler, arguments) and compile(mocFile)) {
        return true;
    }

    return false;
}

void ProjectManager::onTargetName(const QString &target)
{
    qInfo() << "Setting target name:" << target;
    mTargetName = target;
}

void ProjectManager::onTargetType(const QString &type)
{
    qInfo() << "Setting target type:" << type;
    mTargetType = type;
}

void ProjectManager::onQtModules(const QStringList &modules)
{
    QStringList mod(mQtModules);
    mod += modules;
    mod.removeDuplicates();

    if (mod != mQtModules) {
        qInfo() << "Updating required Qt module list:" << mod;
        updateQtModules(mod);
    }
}

void ProjectManager::onIncludes(const QStringList &includes)
{
    mCustomIncludes += includes;
    mCustomIncludes.removeDuplicates();
    qInfo() << "Updating custom includes:" << mCustomIncludes;
}

void ProjectManager::onLibs(const QStringList &libs)
{
    mCustomLibs += libs;
    mCustomLibs.removeDuplicates();
    qInfo() << "Updating custom libs:" << mCustomLibs;
}

bool ProjectManager::compile(const QString &file)
{
    const QFileInfo info(file);
    const QString objectFile(info.baseName() + ".o");
    const QString compiler("g++");

    if (!mQtModules.isEmpty()) {
        if (mQtDir.isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }
    }

    qInfo() << "Compiling:" << file << "into:" << objectFile;
    // TODO: add ProjectManager class and schedule compilation there (threaded!).
    QStringList arguments { "-c", "-pipe", "-g", "-D_REENTRANT", "-fPIC", "-Wall", "-W" };

    arguments.append(mQtDefines);
    arguments.append(mQtIncludes);

    for(const QString &incl : qAsConst(mCustomIncludes)) {
        arguments.append("-I" + incl);
    }

    arguments.append({ "-o", objectFile, file });

    if (runProcess(compiler, arguments)) {
        mObjectFiles.append(objectFile);
        return true;
    }

    return false;
}

bool ProjectManager::link() const
{
    qInfo() << "Linking:" << mObjectFiles;
    const QString compiler("g++");
    QStringList arguments;

    if (mTargetType == Tags::targetLib) {
        if (mTargetLibType == Tags::targetLibDynamic) {
            arguments.append({ "-shared", "-Wl,-soname,lib" + mTargetName + ".so.1",
                               "-o", "lib" + mTargetName + ".so.1.0.0"});
        }
    } else {
        arguments.append({ "-o", mTargetName });
    }

    arguments.append(mObjectFiles);

    if (!mQtModules.isEmpty()) {
        if (mQtDir.isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }

        arguments.append(mQtLibs);
    }

    arguments.append(mCustomLibs);

    return runProcess(compiler, arguments);
}

void ProjectManager::updateQtModules(const QStringList &modules)
{
    mQtModules = modules;
    mQtIncludes.clear();
    mQtLibs.clear();
    mQtDefines.clear();

    for(const QString &module : qAsConst(mQtModules)) {
        mQtDefines.append("-DQT_" + module.toUpper() + "_LIB");
    }

    mQtIncludes.append("-I" + mQtDir + "/include");
    mQtIncludes.append("-I" + mQtDir + "/mkspecs/linux-g++");

    for(const QString &module : qAsConst(mQtModules)) {
        mQtIncludes.append("-I" + mQtDir + "/include/Qt"
                           + capitalizeFirstLetter(module));
    }

    mQtLibs.append("-Wl,-rpath," + mQtDir + "/lib");
    mQtLibs.append("-L" + mQtDir + "/lib");

    for(const QString &module : qAsConst(mQtModules)) {
        // TODO: use correct mkspecs
        // TODO: use qmake -query to get good paths
        mQtLibs.append("-lQt5" + capitalizeFirstLetter(module));
    }

    mQtLibs.append("-lpthread");
}

bool ProjectManager::initializeMoc()
{
    qInfo() << "Initializig MOC";
    const QString compiler("g++");
    const QStringList arguments({ "-pipe", "-g", "-Wall", "-W", "-dM", "-E",
                            "-o", "moc_predefs.h",
                            mQtDir + "/mkspecs/features/data/dummy.cpp" });
    mQtIsMocInitialized = runProcess(compiler, arguments);
    return mQtIsMocInitialized;
}

/*!
 * TODO: run asynchronously in a thread pool.
 */
bool ProjectManager::runProcess(const QString &app, const QStringList &arguments) const
{
    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    qDebug() << "Running:" << app << arguments.join(" ");
    process.start(app, arguments);
    process.waitForFinished(5000);
    const int exitCode = process.exitCode();
    if (exitCode == 0) {
        return true;
    }

    qDebug() << "Process error:" << process.errorString() << exitCode;
    return false;
}

QString ProjectManager::capitalizeFirstLetter(const QString &string) const
{
    return (string[0].toUpper() + string.mid(1));
}
