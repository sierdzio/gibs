#include "metaprocess.h"

#include <QProcess>
#include <QDebug>

MetaProcess::MetaProcess()
{

}

bool MetaProcess::canRun() const
{
    for (const auto &processptr : qAsConst(fileDependencies)) {
        if (!processptr.isNull()) {
            qDebug() << "Waiting for:" << processptr->arguments().last();
            return false;
        }
    }

    return true;
}
