#include "fileparser.h"

#include <QFile>
#include <QFileInfo>

// TODO: add categorized logging!
#include <QDebug>

FileParser::FileParser(const QString &file, QObject *parent) : QObject(parent),
    mFile(file)
{    
}

bool FileParser::parse() const
{
    QFile file(mFile);
    if (mFile.isEmpty() or !file.exists()) {
        qWarning() << "File" << mFile << "does not exist";
        return false;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "File" << mFile << "could not be opened for reading!";
        return false;
    }

    qInfo() << "Parsing:" << mFile;

    QString source;

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line(in.readLine());
        // TODO: add comment and scope detection
        // TODO: add ifdef detection
        if (line.contains("#include")) {
            if (line.contains('<')) {
                // Library include - skip it
            } else if (line.contains('"')) {
                // Local include - parse it!
                QString include(line.mid(line.indexOf('"') + 1));
                include.chop(1);

                emit parseRequest(include);
            }
        }

        // Override default source file location or name
        const QString sourceString("source ");
        if (line.contains(sourceString)) {
            source = line.mid(line.indexOf(sourceString) + sourceString.length());
        }
    }

    const QFileInfo header(mFile);
    if (source.isEmpty() and header.suffix() == "cpp") {
        source = mFile;
    }

    // Guess source file name (.cpp)
    if (source.isEmpty()) {
        if (header.suffix() == "h" or header.suffix() == "hpp") {
            source = header.baseName() + ".cpp";
        }
    }

    // Important: this emit needs to be sent before parseRequest()
    emit parsed(mFile, source);

    // Parse source file, only when we are not parsing it already
    if (!source.isEmpty() and source != mFile) {
        emit parseRequest(source);
    }

    return true;
}
