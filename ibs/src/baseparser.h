#pragma once

#include <QObject>

class BaseParser : public QObject
{
    Q_OBJECT
public:
    explicit BaseParser(QObject *parent = nullptr);

signals:
    void error(const QString &error) const;
    void targetName(const QString &target) const;
    void targetType(const QString &type) const;
    void qtModules(const QStringList &modules) const;
    void defines(const QStringList &defines) const;
    void includes(const QStringList &includes) const;
    void libs(const QStringList &libs) const;
    void runTool(const QString &tool, const QStringList &args) const;

public slots:
    virtual bool parse() const = 0;

protected:
    bool parseCommand(const QString &commandString) const;
    QString extractArguments(const QString &line, const QLatin1String &tag) const;
};
