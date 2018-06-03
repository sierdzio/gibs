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
    void targetLibType(const QString &libType) const;
    void qtModules(const QStringList &modules) const;
    void defines(const QStringList &defines) const;
    void includes(const QStringList &includes) const;
    void libs(const QStringList &libs) const;
    void runTool(const QString &tool, const QStringList &args) const;
    void subproject(const QByteArray &scopeId, const QString &path) const;
    void scopeUpdated(const Scope &scope) const;

public slots:
    virtual bool parse() = 0;

protected:
    bool parseCommand(const QString &commandString);
    QString extractArguments(const QString &line, const QLatin1String &tag) const;
    QString tag(const QLatin1String &rawTag) const;

    QPointer<Scope> mScope; // TODO: maybe BaseParser can inherit from Scope?
};
