#pragma once

#include "scope.h"

#include <QObject>

class BaseParser : public QObject
{
    Q_OBJECT
public:
    explicit BaseParser(const Scope &scope, QObject *parent = nullptr);

signals:
    void error(const QString &error) const;
    void targetName(const QString &target) const;
    void targetType(const QString &type) const;
    void qtModules(const QStringList &modules) const;
    void defines(const QStringList &defines) const;
    void includes(const QStringList &includes) const;
    void libs(const QStringList &libs) const;
    void runTool(const QString &tool, const QStringList &args) const;
    void scopeUpdated(const Scope &scope);

public slots:
    virtual bool parse() = 0;

protected:
    bool parseCommand(const QString &commandString);
    QString extractArguments(const QString &line, const QLatin1String &tag) const;

    Scope mScope;
};
