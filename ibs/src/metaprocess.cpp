#include "metaprocess.h"

#include <QProcess>
#include <QDebug>

MetaProcess::MetaProcess()
{

}

bool MetaProcess::canRun() const
{
    for (auto processptr : qAsConst(dependsOn)) {
        if (!processptr.isNull()) {
            qDebug() << "Waiting for:" << processptr->arguments().last();
            return false;
        }
    }

    return true;
}
