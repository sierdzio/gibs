#pragma once

#include <QString>
#include <QThread>

struct Flags
{
    Flags(const bool _run, const bool _clean, const bool _quick,
          const int _jobs,
          const QString &_qtDir, const QString &_inputFile)
        : run(_run), clean(_clean), quickMode(_quick), jobs(_jobs),
          qtDir(_qtDir), inputFile(_inputFile)
    {
        // No job cap specified, use max number of threads available
        if (jobs == 0) {
            jobs = QThread::idealThreadCount();
        }
    }

    const bool run = false;
    const bool clean = false;
    const bool quickMode = false;

    int jobs = 0;

    const QString qtDir;
    const QString inputFile;
};
