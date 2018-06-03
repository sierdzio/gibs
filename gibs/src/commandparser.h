#pragma once

#include "baseparser.h"

#include <QObject>

class CommandParser : public BaseParser
{
    Q_OBJECT
public:
    explicit CommandParser(const QString &commands, Scope *scope, QObject *parent = nullptr);

public slots:
    bool parse() override final;

private:
    QString mCommandsRaw;
};
