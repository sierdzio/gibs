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
    const bool result = parser.parse();

    // TODO: stop when result is false
    Q_UNUSED(result);
}

bool ProjectManager::compile(const QString &file)
{
    const QFileInfo info(file);
    const QString objectFile(info.baseName() + ".o");
    const QString compiler("g++");

    qInfo() << "Compiling:" << file << "into:" << objectFile;
    // TODO: add ProjectManager class and schedule compilation there (threaded!).
    QStringList arguments { "-c", "-pipe", "-g", "-fPIC", "-Wall", "-W",
        "-o", objectFile, file };

    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    qDebug() << "Compiling:" << compiler << arguments;
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
    QStringList arguments { "-o", mProjectName };

    for (const auto &file : mObjectFiles) {
        arguments.append(file);
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    qDebug() << "Compiling:" << compiler << arguments;
    process.start(compiler, arguments);
    process.waitForFinished(5000);

    const int exitCode = process.exitCode();
    if (exitCode == 0) {
        return true;
    }

    qDebug() << "Process error:" << process.errorString() << exitCode;
    return false;
}
