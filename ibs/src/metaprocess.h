#pragma once

#include <QString>
#include <QVector>
#include <QSharedPointer>

class QProcess;
class MetaProcess;

using ProcessPtr = QSharedPointer<QProcess>;
using MetaProcessPtr = QSharedPointer<MetaProcess>;

class MetaProcess
{
public:
    MetaProcess();

    bool canRun() const;

    bool hasFinished = false;
    QString file; //! Target file (which will be compiled, linked etc.)
    ProcessPtr process; //! QProcess pointer
    QVector<MetaProcessPtr> fileDependencies; //! List of processes which need to end before this one starts
    QVector<QByteArray> scopeDepenencies; //! List of other scopes which this process depends on
};
