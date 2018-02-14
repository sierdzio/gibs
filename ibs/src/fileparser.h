#pragma once

#include <QObject>

class FileParser : public QObject
{
    Q_OBJECT
public:
    explicit FileParser(const QString &file,
                        const QStringList &includeDirs = QStringList(),
                        QObject *parent = nullptr);

signals:
    void parsed(const QString &file, const QString &sourceFile) const;
    void parseRequest(const QString &file) const;
    void runMoc(const QString &file) const;

    // IBS syntax detection:
    void targetName(const QString &target) const;
    void targetType(const QString &type) const;
    void qtModules(const QStringList &modules) const;
    void defines(const QStringList &defines) const;
    void includes(const QStringList &includes) const;
    void libs(const QStringList &libs) const;
    void runTool(const QString &tool, const QStringList &args) const;

public slots:
    bool parse() const;

protected:
    QString extractArguments(const QString &line, const QLatin1String &tag) const;
    QString findFileExtension(const QString &filePath) const;

    const QString mFile;
    QStringList mIncludeDirs;
};
