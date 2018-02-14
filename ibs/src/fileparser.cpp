#include "fileparser.h"
#include "tags.h"

#include <QFile>
#include <QFileInfo>

// TODO: add categorized logging!
#include <QDebug>

FileParser::FileParser(const QString &file, const QStringList &includeDirs, QObject *parent) : QObject(parent),
    mFile(file),
    mIncludeDirs(includeDirs)
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

            // TODO: handle spaces in target name
            if(line.contains(Tags::targetCommand)) {
                if (line.contains(Tags::targetName)) {
                    const QString arg(extractArguments(line, Tags::targetName));
                    qDebug() << "Target name:" << arg;
                    emit targetName(arg);
                }

                if (line.contains(Tags::targetType)) {
                    const QString arg(extractArguments(line, Tags::targetType));

                    if (arg == Tags::targetApp || arg == Tags::targetLib) {
                        qDebug() << "Target type:" << arg;
                        emit targetType(arg);
                    } else {
                        qFatal("Invalid target type: %s", qPrintable(arg));
                    }
                }
            }

            if (line.contains(Tags::qtModules)) {
                const QStringList args(extractArguments(line, Tags::qtModules).split(" "));
                qDebug() << "Enabling Qt modules:" << args;
                emit qtModules(args);
            }

            if (line.contains(Tags::defines)) {
                const QStringList args(extractArguments(line, Tags::defines).split(" "));
                qDebug() << "Adding defines:" << args;
                emit defines(args);
            }

            if (line.contains(Tags::includes)) {
                const QStringList args(extractArguments(line, Tags::includes).split(" "));
                qDebug() << "Adding includes:" << args;
                emit includes(args);
            }

            if (line.contains(Tags::libs)) {
                const QStringList args(extractArguments(line, Tags::libs).split(" "));
                qDebug() << "Adding libs:" << args;
                emit libs(args);
            }

            if (line.contains(Tags::tool)) {
                const QStringList args(extractArguments(line, Tags::tool).split(" "));
                qDebug() << "Running tool:" << args;
                if (args.size() > 0) {
                    emit runTool(args.at(0), args.mid(1));
                }
            }
        }
    }

    const QFileInfo header(mFile);
    if (source.isEmpty() and (header.suffix() == "cpp" or header.suffix() == "c"
                              or header.suffix() == "cc")) {
        source = mFile;
    }

    // Guess source file name (.cpp)
    if (source.isEmpty()) {
        if (header.suffix() == "h" or header.suffix() == "hpp") {
            const QString base(header.path() + "/" + header.baseName());
            QString ext(findFileExtension(base));

            // Search through include paths
            if (ext.isEmpty()) {
                for (const QString &inc : qAsConst(mIncludeDirs)) {
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
        emit parsed(mFile, source);
    else
        emit parsed(mFile, QString());

    // Parse source file, only when we are not parsing it already
    if (!source.isEmpty() and source != mFile) {
        emit parseRequest(source);
    }

    return true;
}

QString FileParser::extractArguments(const QString &line, const QLatin1String &tag) const
{
    return line.mid(line.indexOf(tag) + tag.size() + 1);
}

QString FileParser::findFileExtension(const QString &filePath) const
{
    if (QFileInfo(filePath + ".cpp").exists()) return ".cpp";
    if (QFileInfo(filePath + ".c").exists()) return ".c";
    if (QFileInfo(filePath + ".cc").exists()) return ".cc";
    return QString();
}
