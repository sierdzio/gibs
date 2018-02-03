#include "fileparser.h"

#include <QFile>

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
                QString include(line.mid(line.indexOf('"')));
                include.chop(1);

                FileParser parser(include);
                connect(&parser, &FileParser::parsed, this, &FileParser::parsed);
                parser.parse();
            }
        }
    }

    emit parsed(mFile);
    return true;
}
