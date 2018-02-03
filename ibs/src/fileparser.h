#pragma once

#include <QObject>

class FileParser : public QObject
{
    Q_OBJECT
public:
    explicit FileParser(const QString &file, QObject *parent = nullptr);

signals:
    void parsed(const QString &file, const QString &sourceFile) const;
    void parseRequest(const QString &file) const;

public slots:
    bool parse() const;

protected:
    const QString mFile;
};
