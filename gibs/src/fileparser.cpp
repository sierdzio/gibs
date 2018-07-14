#include "fileparser.h"
#include "tags.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>

// TODO: add categorized logging!
#include <QDebug>

FileParser::FileParser(const QString &file, const bool parseWholeFiles,
                       Scope *scope, BaseParser *parent)
    : BaseParser (scope, parent),
      mParseWholeFiles(parseWholeFiles),
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

    ParseBlock block;
    block.active = block.defined;
    QString source;
    QByteArray rawContents;
    QCryptographicHash checksum(QCryptographicHash::Sha1);
    // Cache:
    int scopeFeatureCount = mScope->features().count();
    int previousFeatureCount = 0;

    //QTextStream in(&file);
    while (!file.atEnd()) {
        // We remove any leading and trailing whitespace for simplicity
        const QByteArray rawLine(file.readLine());
        const QString line(rawLine.trimmed());

        if (mParseWholeFiles == true) {
            // TODO: what to do with checksum if we are not parsing whole files?
            // Most probably this needs to be removed.
            checksum.addData(rawLine);
            // TODO: use separate flag for saving whole file data
            rawContents.append(rawLine);
        } else {
            // TODO: make "real code" detector more robust
            if (line.contains("::") or line.contains(" class "))
                break;
        }

        // TODO: use clang to detects blocks properly. Or write a proper lexer
        // Detect block
        if (line.startsWith('#')) {
            // If this is an ifdef line, we don't need to do any further parsing
            bool skipParsing = true;
            const QStringList w(line.split(' ', QString::SkipEmptyParts));
            QStringList words;

            // TODO: if new define was added, we should update block.active
            // and block.defined here!
            scopeFeatureCount = mScope->features().count();
            if (scopeFeatureCount != previousFeatureCount) {
                previousFeatureCount = scopeFeatureCount;
                const auto &features = mScope->features().values();
                for (const auto &feature : features) {
                    if (!block.defined.contains(feature.define)) {
                        block.defined.insert(feature.define, feature.enabled);
                    }
                }
            }

            // Glue words together
            for (int i = 0; i < w.length(); ++i) {
                // TODO: support line breaks by backslashes
                if ((i < w.length() - 1) and
                        (w.at(i) == '#' or w.at(i) == '!'
                         or w.at(i) == "defined" or w.at(i) == "("))
                {
                    words.append(w.at(i) + w.at(i + 1));
                    ++i;
                    continue;
                }

                words.append(w.at(i));
            }

            if (words.at(0) == "#ifdef" or words.at(0) == "#elif") {
                // #ifdef STH
                // #ifdef ! STH
                const QStringList keys(block.active.keys());
                for (const QString &key : keys) {
                    block.active.insert(key, false);
                }

                if (words.at(1).startsWith('!') == false)
                    block.active.insert(words.at(1), true);
            } else if (words.at(0) == "#if") {
                // #if defined(STH)
            } else if (words.at(0) == "#else") {
                const QStringList keys(block.active.keys());
                for (const QString &key : keys) {
                    block.active.insert(key, false);
                }
            } else if (words.at(0) == "#endif") {
                // Reset active blocks
                block.active = block.defined;
            } else {
                skipParsing = false;
            }

            if (skipParsing) {
                continue;
            }
        }

        //qDebug() << "Blocks: Active:" << block.active << "### Defined:" << block.defined;

        const bool canRead = canReadIncludes(block);

        // TODO: add comment and scope detection
        if (line.startsWith("#include")) {
            if (line.contains('<')) {
                // Library include - skip it
            } else if (line.contains('"')) {
                if (canRead) {
                    // Local include - parse it!
                    QString include(line.mid(line.indexOf('"') + 1));
                    include.chop(1);

                    emit parseRequest(include, false);
                }
            }
        }

        if (line.startsWith("Q_OBJECT") or line.startsWith("Q_GADGET")) {
            emit runMoc(mFile);
        }

        // Detect GIBS comment scope
        if (scopeBegins(line))
            block.isComment = true;
        if (scopeEnds(line, block))
            block.isComment = false;

        // Handle GIBS comments (commands)
        if (line.startsWith(Tags::scopeOneLine + " ") or block.isComment) {
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
                const auto includePaths = mScope->includePaths();
                for (const QString &inc : includePaths) {
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
    if (QFileInfo::exists(source)) {
        emit parsed(mFile, source, checksum.result(),
                    header.lastModified(), header.created(),
                    rawContents);
    } else {
        emit parsed(mFile, QString(), checksum.result(),
                    header.lastModified(), header.created(),
                    rawContents);
    }

    // Parse source file, only when we are not parsing it already
    if (!source.isEmpty() and source != mFile) {
        emit parseRequest(source, true);
    }

    return true;
}

QString FileParser::findFileExtension(const QString &filePath) const
{
    if (QFileInfo::exists(filePath + ".cpp")) return ".cpp";
    if (QFileInfo::exists(filePath + ".c")) return ".c";
    if (QFileInfo::exists(filePath + ".cc")) return ".cc";
    return QString();
}

bool FileParser::scopeBegins(const QString &line) const
{
    return (line == Tags::scopeBegin or line.startsWith(Tags::scopeBegin + " "));
}

bool FileParser::scopeEnds(const QString &line, const ParseBlock &block) const
{
    return (block.isComment and line.contains(Tags::scopeEnd));
}

/*!
 * Returns true if ifdefs set in current \a block allow includes to be read.
 */
bool FileParser::canReadIncludes(const ParseBlock &block) const
{
    const QList<QString> activeKeys(block.active.keys(true));
    for (const QString &key : activeKeys) {
        if (block.defined.value(key, false) == true) {
            return true;
        }
    }

    return false;
}
