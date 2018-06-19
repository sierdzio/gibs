#pragma once

#include <QString>
#include <QThread>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

#include <cmath>

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
    void setClean(const bool clean)
    {
        mClean = clean;
    }

    bool quickMode() const
    {
        return mQuickMode;
    }
    void setQuickMode(const bool quickMode)
    {
        mQuickMode = quickMode;
    }

    int jobs() const
    {
        return mJobs;
    }

    void setJobs(const float jobs)
    {
        if (qFuzzyCompare(qreal(jobs), 0.0) or std::isless(jobs, 0.0)) {
            // No job cap specified, use max number of threads available
            mJobs = QThread::idealThreadCount();
        } else if (std::isless(jobs, 1.00)) {
            // Cap will be interpreted as percentage
            const int maxCount = QThread::idealThreadCount();
            mJobs = std::floor(qreal(maxCount) * jobs);
        } else {
            mJobs = std::ceil(jobs);
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
        //qDebug() << "Set relative path to:" << mRelativePath
        //         << "FROM" << inputFile;
    }

    QString prefix() const
    {
        return mPrefix;
    }

    void setPrefix(const QString &prefix)
    {
        const QDir prefixDir(prefix);

        if (!prefixDir.exists()) {
            qFatal("Prefix directory does not exist. Bailing out!");
        }

        mPrefix = prefix;
    }

    QString toString() const
    {
        QString result("");
        return result;
    }

    bool qtAutoModules() const
    {
        return mQtAutoModules;
    }

    void setQtAutoModules(const bool qtAutoModules)
    {
        mQtAutoModules = qtAutoModules;
    }

    bool autoIncludes() const
    {
        return mAutoIncludes;
    }

    void setAutoIncludes(const bool autoIncludes)
    {
        mAutoIncludes = autoIncludes;
    }

    bool parseWholeFiles() const
    {
        return mParseWholeFiles;
    }

    void setParseWholeFiles(bool parseWholeFiles)
    {
        mParseWholeFiles = parseWholeFiles;
    }

private:
    bool mRun = false;
    bool mClean = false;
    bool mQuickMode = false;
    bool mQtAutoModules = false;
    bool mAutoIncludes = false;
    bool mParseWholeFiles = false;

    int mJobs = 0;

    QString mQtDir;
    QString mInputFile;

    // Gibs commands passed on the command line
    QString mCommands;

    // Paths
    QString mRelativePath; // pointing to dir where the input file is located
    QString mPrefix = "."; //QDir::currentPath(); // where target will be put (== prefix)
};
