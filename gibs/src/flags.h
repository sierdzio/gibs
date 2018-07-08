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
            mJobs = int(std::floor(qreal(maxCount) * qreal(jobs)));
        } else {
            mJobs = int(std::ceil(jobs));
        }
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

public:
    bool run = false;
    bool clean = false;
    bool quickMode = false;
    bool qtAutoModules = false;
    bool autoIncludes = false;
    bool parseWholeFiles = false;
    bool debugBuild = false;
    bool releaseBuild = true;

    QString qtDir;
    QString inputFile;

    // Gibs commands passed on the command line
    QString commands;

    // Deployment
    QString deployerName;
    QString deployerPath;

    // Compilation
    QString compilerName;

    // Cross compilation
    bool crossCompile = false;
    QString sysroot; ///media/sierdzio/data/android/ndk-r15/sysroot
    QString toolchain;
    QString androidNdkPath;
    QString androidNdkApi;
    QString androidNdkAbi;
    QString androidSdkPath;
    QString androidSdkApi;
    QString jdkPath = "/usr/lib/jvm/java-8-openjdk-amd64";

private:
    int mJobs = 0;

    // Paths
    QString mRelativePath; // pointing to dir where the input file is located
    QString mPrefix = "."; //QDir::currentPath(); // where target will be put (== prefix)
};
