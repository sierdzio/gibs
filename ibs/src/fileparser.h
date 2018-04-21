#pragma once

#include "baseparser.h"

#include <QObject>

class FileParser : public BaseParser
{
    Q_OBJECT
public:
    explicit FileParser(const QString &file, Scope *scope,
                        BaseParser *parent = nullptr);

signals:
    void parsed(const QByteArray &scopeId,
                const QString &file,
                const QString &sourceFile,
                const QByteArray &checksum,
                const QDateTime &modified,
                const QDateTime &created) const;
    void parseRequest(const QByteArray &scopeId,
                      const QString &file,
                      const bool force) const;
    void runMoc(const QByteArray &scopeId,
                const QString &file) const;

public slots:
    bool parse() override final;

protected:
    QString findFileExtension(const QString &filePath) const;

    const QString mFile;
};
