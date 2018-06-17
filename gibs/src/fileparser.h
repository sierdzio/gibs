#pragma once

#include "baseparser.h"

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QObject>

struct ParseBlock {
    //! Set to true when current line is inside a gibs comment block
    bool isComment = false;
    //! Set to true when current line is inside a disabled block (inside an
    //! ifdef for example)
    bool isDisabled = false;
};

class FileParser : public BaseParser
{
    Q_OBJECT
public:
    explicit FileParser(const QString &file, Scope *scope,
                        BaseParser *parent = nullptr);

signals:
    void parsed(const QString &file,
                const QString &sourceFile,
                const QByteArray &checksum,
                const QDateTime &modified,
                const QDateTime &created) const;
    void parseRequest(const QString &file,
                      const bool force) const;
    void runMoc(const QString &file) const;

public slots:
    bool parse() override final;

protected:
    QString findFileExtension(const QString &filePath) const;
    bool scopeBegins(const QString &line) const;
    bool scopeEnds(const QString &line, const ParseBlock &block) const;

    const QString mFile;
};
