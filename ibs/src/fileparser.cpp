#include "fileparser.h"
#include "tags.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>

// TODO: add categorized logging!
#include <QDebug>

FileParser::FileParser(const Scope &scope, const QString &file, BaseParser *parent)
    : BaseParser (scope, parent),
      mFile(file)
{    
}

bool FileParser::parse()
 {
    QFile file(mFile);
    if (mFile.isEmpty() or !file.exists()) {
        emit error(QString("File %1 does not exist").arg(mFile));
        return false;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit error(QString("File %1 could not be opened for reading!").arg(mFile));
        return false;
    }

    qInfo() << "Parsing:" << mFile;

    bool isCommentScope = false;
    QString source;
    QCryptographicHash checksum(QCryptographicHash::Sha1);

    //QTextStream in(&file);
    while (!file.atEnd()) {
        // We remove any leading and trailing whitespace for simplicity
        const QByteArray rawLine(file.readLine());
        const QString line(rawLine.trimmed());

        checksum.addData(rawLine);

        // TODO: add comment and scope detection
        // TODO: add ifdef detection
        if (line.startsWith("#include")) {
            if (line.contains('<')) {
                // Library include - skip it
            } else if (line.contains('"')) {
                // Local include - parse it!
                QString include(line.mid(line.indexOf('"') + 1));
                include.chop(1);

                emit parseRequest(include, false);
            }
        }

        if (line.startsWith("Q_OBJECT") or line.startsWith("Q_GADGET")) {
            emit runMoc(mFile);
        }

        // Detect IBS comment scope
        if (line == Tags::scopeBegin or line.startsWith(Tags::scopeBegin + " "))
            isCommentScope = true;
        if (isCommentScope and line.contains(Tags::scopeEnd))
            isCommentScope = false;

        // Handle IBS comments (commands)
        if (line.startsWith(Tags::scopeOneLine) or isCommentScope) {
            // Override default source file location or name
            if (line.contains(Tags::source))
                source = extractArguments(line, Tags::source);

            parseCommand(line);
        }
    }

    const QFileInfo header(mFile);
    if (source.isEmpty() and (header.suffix() == "cpp" or header.suffix() == "c"
                              or header.suffix() == "cc"))
    {
        source = mFile;
    }

    // Guess source file name (.cpp)
    if (source.isEmpty()) {
        if (header.suffix() == "h" or header.suffix() == "hpp") {
            const QString base(header.path() + "/" + header.baseName());
            QString ext(findFileExtension(base));

            // Search through include paths
            if (ext.isEmpty()) {
                for (const QString &inc : mScope.includePaths()) {
                    const QString incBase(inc + "/" + header.baseName());
                    ext = findFileExtension(incBase);
                    if (!ext.isEmpty()) {
                        qDebug() << "Found source file in include paths!" << source;
                        source = incBase + ext;
                        break;
                    }
                }
            } else {
                source = base + ext;
            }
        }
    }

    // Important: this emit needs to be sent before parseRequest()
    if (QFileInfo(source).exists())
        emit parsed(mFile, source, checksum.result(), header.lastModified(), header.created());
    else
        emit parsed(mFile, QString(), checksum.result(), header.lastModified(), header.created());

    // Parse source file, only when we are not parsing it already
    if (!source.isEmpty() and source != mFile) {
        emit parseRequest(source, true);
    }

    return true;
}

QString FileParser::findFileExtension(const QString &filePath) const
{
    if (QFileInfo(filePath + ".cpp").exists()) return ".cpp";
    if (QFileInfo(filePath + ".c").exists()) return ".c";
    if (QFileInfo(filePath + ".cc").exists()) return ".cc";
    return QString();
}
