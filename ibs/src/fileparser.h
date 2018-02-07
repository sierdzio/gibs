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

    // IBS syntax detection:
    void targetName(const QString &target) const;
    void qtModules(const QStringList &modules) const;
    void includes(const QStringList &includes) const;
    void libs(const QStringList &libs) const;

public slots:
    bool parse() const;

protected:
    QString extractArguments(const QString &line, const QLatin1String &tag) const;

    const QString mFile;
};
