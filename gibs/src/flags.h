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

/*!
 * \class Flags
 * \brief Holds flags and configuration options set via command line
 *
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
 *
 * The following flag values are stored between gibs runs:
 * - qtDir
 * - deployerPath
 * - sysroot
 * - toolchain
 * - androidNdkPath
 * - androidNdkApi
 * - androidNdkAbi
 * - androidSdkPath
 * - androidSdkApi
 * - jdkPath
 */
class Flags : public MConfig
{
public:
    Flags(const bool usePathConfig = true) : MConfig("PathConfig") {
        // These values are stored between application run.
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

    /*!
     * Saves the remembered flags to gibs config file (global).
     */
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

    /*!
     * Returns the number of jobs (threads) that will be used for building the
     * code. This is the number arrived at after processing `-j` flag in
     * setJobs().
     */
    int jobs() const
    {
        return mJobs;
    }

    /*!
     * Determines how many jobs (threads) should be used for building based on
     * \a jobs value.
     *
     * If \a jobs is larger than 1, it denotes how many threads will be used for
     * builing the source code in parallel.
     *
     * If \a jobs is smaller than 1, it denotes which percentage of available
     * CPU cores should be used for building the source code in parallel. For
     * example, using `-j 0.5` will use 50% of CPU cores. If your CPU has 4 cores,
     * gibs will compile using 2 of them then.
     */
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

    /*!
     * Returns the relative path between current dir and root directory of the
     * source code being built.
     *
     * For example, if you call `gibs ../someproject/main.cpp`, the relative
     * path will be `../someproject/`.
     */
    QString relativePath() const
    {
        return mRelativePath;
    }

    /*!
     * Sets the relative path based on \a inputFile.
     *
     * \sa relativePath
     */
    void setRelativePath(const QString &inputFile)
    {
        const QFileInfo info(inputFile);
        mRelativePath = info.path();
        //qDebug() << "Set relative path to:" << mRelativePath
        //         << "FROM" << inputFile;
    }

    /*!
     * Returns the prefix directory - the directory where build code will be
     * placed (yes, at this point that includes the object files and other
     * build artifacts).
     *
     * \todo use prefix dir only to put the resulting binaries there.
     */
    QString prefix() const
    {
        return mPrefix;
    }

    /*!
     * Sets the prefix directory to \a prefix.
     *
     * \sa prefix
     */
    void setPrefix(const QString &prefix)
    {
        const QDir prefixDir(prefix);

        if (!prefixDir.exists()) {
            qFatal("Prefix directory does not exist. Bailing out!");
        }

        mPrefix = prefix;
    }    

    /*!
     * Returns true if gibs will pipe source code into the compiler.
     *
     * If yes, gibs will read the source code once only from the hard drive,
     * the compiler won't need to to it again.
     */
    bool pipe() const
    {
        return pipeFlag;
    }

    /*!
     * Sets whether gibs should pipe source code into the compiler or not.
     *
     * \sa pipe
     */
    void setPipe(bool value)
    {
        pipeFlag = value;
        if (pipeFlag == true)
            parseWholeFiles = true;
    }

    /*!
     * NOT IMPLEMENTED. Returns values of all flags in a single, printable string.
     */
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
    bool pipeFlag = false;
    int mJobs = 0;

    // Paths
    QString mRelativePath; // pointing to dir where the input file is located
    QString mPrefix = "."; //QDir::currentPath(); // where target will be put (== prefix)
};
