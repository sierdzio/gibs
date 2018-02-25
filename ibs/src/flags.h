#pragma once

#include <QString>

struct Flags
{
    Flags(const bool _run, const bool _clean, const bool _quick,
          const QString &_qtDir, const QString &_inputFile)
        : run(_run), clean(_clean), quickMode(_quick),
          qtDir(_qtDir), inputFile(_inputFile)
    {
    }

    const bool run;
    const bool clean;
    const bool quickMode;

    const QString qtDir;
    const QString inputFile;
};
