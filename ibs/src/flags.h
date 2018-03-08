#pragma once

#include <QString>

struct Flags
{
    Flags(const bool _run, const bool _clean, const bool _quick,
          const int _jobs,
          const QString &_qtDir, const QString &_inputFile)
        : run(_run), clean(_clean), quickMode(_quick), jobs(_jobs),
          qtDir(_qtDir), inputFile(_inputFile)
    {
    }

    const bool run = false;
    const bool clean = false;
    const bool quickMode = false;

    const int jobs = 1;

    const QString qtDir;
    const QString inputFile;
};
