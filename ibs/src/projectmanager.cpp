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
    mParsedFiles.append(file);

    if (!source.isEmpty() and source == file) {
        compile(source);
    }
}

void ProjectManager::onParseRequest(const QString &file)
{
    // Skip files which we have parsed already
    if (mParsedFiles.contains(file))
        return;

    FileParser parser(file);
    connect(&parser, &FileParser::parsed, this, &ProjectManager::onParsed);
    connect(&parser, &FileParser::parseRequest, this, &ProjectManager::onParseRequest);
    connect(&parser, &FileParser::qtModules, this, &ProjectManager::onQtModules);
    connect(&parser, &FileParser::targetName, this, [=](const QString &targetName)
        { mTargetName = targetName; });
    const bool result = parser.parse();

    // TODO: stop when result is false
    Q_UNUSED(result);
}

void ProjectManager::onQtModules(const QStringList &modules)
{
    mQtModules += modules;
    mQtModules.removeDuplicates();
    qInfo() << "Updating required Qt module list:" << mQtModules;
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

    for(const QString &module : mQtModules) {
        arguments.append("-DQT_" + module.toUpper() + "_LIB");
    }

    // TODO: use correct mkspecs
    // TODO: use qmake -query to get good paths
    arguments.append("-I" + mQtDir + "/include");
    arguments.append("-I" + mQtDir + "/mkspecs/linux-g++");

    for(const QString &module : mQtModules) {
        QString Module(module);
        Module[0] = Module.at(0).toUpper();
        arguments.append("-I" + mQtDir + "/include/Qt" + Module);
    }

    arguments.append({ "-o", objectFile, file });

    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    qDebug() << "Compiling:" << compiler << arguments.join(" ");
    process.start(compiler, arguments);
    process.waitForFinished(5000);

    const int exitCode = process.exitCode();
    if (exitCode == 0) {
        mObjectFiles.append(objectFile);
        return true;
    }

    qDebug() << "Process error:" << process.errorString() << exitCode;
    return false;
}

bool ProjectManager::link()
{
    qInfo() << "Linking:" << mObjectFiles;
    const QString compiler("g++");
    QStringList arguments { "-o", mTargetName };

    for (const auto &file : mObjectFiles) {
        arguments.append(file);
    }

    if (!mQtModules.isEmpty()) {
        if (mQtDir.isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }

        arguments.append("-Wl,-rpath," + mQtDir + "/lib");
        arguments.append("-L" + mQtDir + "/lib");

        for(const QString &module : mQtModules) {
            // TODO: use correct mkspecs
            // TODO: use qmake -query to get good paths
            // Capitalize first letter. Horrible solution, but will do for now
            QString Module(module);
            Module[0] = Module.at(0).toUpper();
            arguments.append("-lQt5" + Module);
        }

        arguments.append("-lpthread");
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    qDebug() << "Linking:" << compiler << arguments.join(" ");
    process.start(compiler, arguments);
    process.waitForFinished(5000);

    const int exitCode = process.exitCode();
    if (exitCode == 0) {
        return true;
    }

    qDebug() << "Process error:" << process.errorString() << exitCode;
    return false;
}
