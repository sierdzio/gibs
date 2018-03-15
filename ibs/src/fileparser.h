#pragma once

#include <QObject>

#include "baseparser.h"

class FileParser : public BaseParser
{
    Q_OBJECT
public:
    explicit FileParser(const QString &file,
                        const QStringList &includeDirs = QStringList(),
                        BaseParser *parent = nullptr);

signals:
    void parsed(const QString &file, const QString &sourceFile,
                const QByteArray &checksum,
                const QDateTime &modified,
                const QDateTime &created) const;
    void parseRequest(const QString &file, const bool force) const;
    void runMoc(const QString &file) const;

public slots:
    bool parse() const override final;

protected:
    QString findFileExtension(const QString &filePath) const;

    const QString mFile;
    QStringList mIncludeDirs;
};
