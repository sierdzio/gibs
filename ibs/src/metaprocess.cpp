#include "metaprocess.h"

MetaProcess::MetaProcess()
{

}

bool MetaProcess::canRun() const
{
    for (auto processptr : qAsConst(dependsOn)) {
        if (!processptr.isNull())
            return false;
    }

    return true;
}
