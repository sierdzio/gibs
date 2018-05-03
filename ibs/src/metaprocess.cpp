#include "metaprocess.h"

#include <QProcess>
#include <QDebug>

MetaProcess::MetaProcess()
{
}

bool MetaProcess::canRun() const
{
    if (process.isNull()) {
        return false;
    }

    for (const auto &metaprocess : qAsConst(fileDependencies)) {
        if (metaprocess->hasFinished == false) {
            qDebug() << "Waiting for:" << metaprocess->process->arguments().last();
            return false;
        }
    }

    // TODO: scope dependencies!

    return true;
}
