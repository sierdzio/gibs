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
    auto fp = new FileParser(mInputFile, this);
    connect(fp, &FileParser::parsed, this, &ProjectManager::onParsed);
    // TODO: make it asynchronous!
    fp->parse();
    // Parsing done, link it!
    link();
}

void ProjectManager::onParsed(const QString &file)
{
    mParsedFiles.append(file);
    compile(file);
}

bool ProjectManager::compile(const QString &file)
{
    qInfo() << "Compiling:" << file;
    const QFileInfo info(file);
    // TODO: add ProjectManager class and schedule compilation there (threaded!).
    QStringList arguments { "-o", info.baseName() + ".o" };

    QProcess process;
    process.start("g++", arguments);
    const QString errorString(process.errorString());
    if (errorString.isEmpty()) {
        return true;
    }

    qDebug() << errorString;
    return false;
}

bool ProjectManager::link()
{
    qInfo() << "Linking:" << mParsedFiles;
    // TODO: add ProjectManager class and schedule compilation there (threaded!).
    QStringList arguments { "-l" };

    QProcess process;
    process.start("g++", arguments);
    const QString errorString(process.errorString());
    if (errorString.isEmpty()) {
        return true;
    }

    qDebug() << errorString;
    return false;
}
