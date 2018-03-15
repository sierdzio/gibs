#pragma once

#include "baseparser.h"

#include <QObject>

class CommandParser : public BaseParser
{
    Q_OBJECT
public:
    explicit CommandParser(const QString &commands, QObject *parent = nullptr);

public slots:
    bool parse() const override final;

private:
    QString mCommandsRaw;
};
