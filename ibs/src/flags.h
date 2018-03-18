#pragma once

#include <QString>
#include <QThread>
#include <QDir>
#include <QFileInfo>

class Flags
{
public:
    Flags()
    {
    }

    bool run() const
    {
        return mRun;
    }

    void setRun(const bool run)
    {
        mRun = run;
    }

    bool clean() const
    {
        return mClean;
    }
    void setClean(bool clean)
    {
        mClean = clean;
    }

    bool quickMode() const
    {
        return mQuickMode;
    }
    void setQuickMode(bool quickMode)
    {
        mQuickMode = quickMode;
    }

    int jobs() const
    {
        return mJobs;
    }
    void setJobs(int jobs)
    {
        mJobs = jobs;
        // No job cap specified, use max number of threads available
        if (mJobs == 0) {
            mJobs = QThread::idealThreadCount();
        }
    }

    QString qtDir() const
    {
        return mQtDir;
    }
    void setQtDir(const QString &qtDir)
    {
        mQtDir = qtDir;
    }

    QString inputFile() const
    {
        return mInputFile;
    }
    void setInputFile(const QString &inputFile)
    {
        mInputFile = inputFile;
    }

    QString commands() const
    {
        return mCommands;
    }

    void setCommands(const QString &commands)
    {
        mCommands = commands;
    }

    QString relativePath() const
    {
        return mRelativePath;
    }

    void setRelativePath(const QString &inputFile)
    {
        const QFileInfo info(inputFile);
        mRelativePath = info.path();
    }

    QString buildPath() const;
    void setBuildPath(const QString &buildPath);

    QString prefix() const;
    void setPrefix(const QString &prefix);

private:
    bool mRun = false;
    bool mClean = false;
    bool mQuickMode = false;

    int mJobs = 0;

    QString mQtDir;
    QString mInputFile;

    // Ibs commands passed on the command line
    QString mCommands;

    // Paths
    QString mRelativePath; // pointing to dir where the input file is located
    QString mBuildPath = QDir::currentPath(); // where build artifacts will be put. Default: current dir
    QString mPrefix = QDir::currentPath(); // where target will be put (== prefix)
};
