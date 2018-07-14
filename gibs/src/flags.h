#pragma once

#include "mconfig.h"
#include "tags.h"

#include <QString>
#include <QThread>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

#include <cmath>

class Flags : public MConfig
{
public:
    /*!
     * Flags acts like runtime memory for flags and switches passed on command
     * line. However, it will also store / read some data in config file.
     *
     * This is done mostly to save typing: you don't need to specify Qt dir,
     * sysroot, Android NDK path etc. each time you call gibs, because it will
     * be remembered between runs.
     *
     * Paths precedence is:
     *
     * 1. Paths passed on command line.
     *
     * 2. Paths saved in current directory.
     *
     * 3. Paths saved in $HOME/.config/gibs
     */
    Flags(const bool usePathConfig = true) : MConfig("PathConfig") {
        CONFIG_VALUE(qtDir, QMetaType::QString);
        CONFIG_VALUE(deployerPath, QMetaType::QString);
        CONFIG_VALUE(sysroot, QMetaType::QString);
        CONFIG_VALUE(toolchain, QMetaType::QString);
        CONFIG_VALUE(androidNdkPath, QMetaType::QString);
        CONFIG_VALUE(androidNdkApi, QMetaType::QString);
        CONFIG_VALUE(androidNdkAbi, QMetaType::QString);
        CONFIG_VALUE(androidSdkPath, QMetaType::QString);
        CONFIG_VALUE(androidSdkApi, QMetaType::QString);
        CONFIG_VALUE(jdkPath, QMetaType::QString);

        if (usePathConfig == false)
            return;

        // TODO: use QDir::setSearchPaths()!

        qDebug() << "Looking for gibs paths config in current dir";
        if (QFileInfo::exists(Tags::gibsConfigFileName)) {
            load(Tags::gibsConfigFileName);
            return;
        }

        const QString standard(QStandardPaths::standardLocations(
            QStandardPaths::ConfigLocation).at(0));
        const QString configFile(standard + "/gibs/" + Tags::gibsConfigFileName);
        qDebug() << "Looking for gibs paths config in:" << configFile;
        if (QFileInfo::exists(configFile)) {
            load(configFile);
            return;
        }

        qDebug() << "None found, will use default values";
    }

    void save() const {
        const QString standard(QStandardPaths::standardLocations(
                               QStandardPaths::ConfigLocation).at(0));
        const QString configDir(standard + "/gibs");
        const QString configFile(configDir + "/" + Tags::gibsConfigFileName);

        const QDir root(QDir::root());
        if (root.exists(configDir) == false)
            root.mkpath(configDir);

        qDebug() << "Writing path config file:" << configFile;
        MConfig::save(configFile);
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

    bool getPipe() const
    {
        return pipe;
    }

    void setPipe(bool value)
    {
        pipe = value;
        if (pipe == true)
            parseWholeFiles = true;
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
    bool pipe = false;

    QString qtDir;
    QString inputFile;

    // Gibs commands passed on the command line
    QString commands;

    // Deployment
    QString deployerName;
    QString deployerPath;

    // Compilation
    QString compilerName = "gcc";

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
