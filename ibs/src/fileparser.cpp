#include "fileparser.h"
#include "tags.h"

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

    bool isCommentScope = false;
    QString source;

    QTextStream in(&file);
    while (!in.atEnd()) {
        // We remove any leading and trailing whitespace for simplicity
        const QString line(in.readLine().trimmed());
        // TODO: add comment and scope detection
        // TODO: add ifdef detection
        if (line.startsWith("#include")) {
            if (line.contains('<')) {
                // Library include - skip it
            } else if (line.contains('"')) {
                // Local include - parse it!
                QString include(line.mid(line.indexOf('"') + 1));
                include.chop(1);

                emit parseRequest(include);
            }
        }

        // Detect IBS comment scope
        if (line.startsWith(Tags::scopeBegin))
            isCommentScope = true;
        if (isCommentScope and line.contains(Tags::scopeEnd))
            isCommentScope = false;

        // Handle IBS comments (commands)
        if (line.startsWith(Tags::scopeOneLine) or isCommentScope) {
            // Override default source file location or name
            if (line.contains(Tags::source))
                source = line.mid(line.indexOf(Tags::source) + Tags::source.size());

            // TODO: handle spaces in target name
            if (line.contains(Tags::targetName))
                emit targetName(line.mid(line.indexOf(Tags::targetName) + Tags::targetName.size()));
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
