#pragma once

#include <QString>
#include <QVector>
#include <QPointer>

class QProcess;

using ProcessPtr = QPointer<QProcess>;

class MetaProcess
{
public:
    MetaProcess();

    bool canRun() const;

    QString file; //! Target file (which will be compiled, linked etc.)
    ProcessPtr process; //! QProcess pointer
    QVector<ProcessPtr> dependsOn; //! List of processes which need to end before this one starts
};
