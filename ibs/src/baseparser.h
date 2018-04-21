#pragma once

#include "scope.h"

#include <QObject>
#include <QPointer>

class BaseParser : public QObject
{
    Q_OBJECT

public:
    explicit BaseParser(Scope *scope, QObject *parent = nullptr);

signals:
    void error(const QString &error) const;
    void targetName(const QString &target) const;
    void targetType(const QString &type) const;
    void qtModules(const QStringList &modules) const;
    void defines(const QStringList &defines) const;
    void includes(const QStringList &includes) const;
    void libs(const QStringList &libs) const;
    void runTool(const QByteArray &scopeId, const QString &tool, const QStringList &args) const;
    void scopeUpdated(const Scope &scope);

public slots:
    virtual bool parse() = 0;

protected:
    bool parseCommand(const QString &commandString);
    QString extractArguments(const QString &line, const QLatin1String &tag) const;

    QPointer<Scope> mScope; // TODO: maybe BaseParser can inherit from Scope?
};
