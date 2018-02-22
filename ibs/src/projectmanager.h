#pragma once

#include <QObject>
#include <QHash>
#include <QDateTime>

#include "tags.h"

class QJsonArray;

struct FileInfo {
    QString path;
    QDateTime dateModified;
    QDateTime dateCreated;
    QByteArray checksum;

    QJsonArray toJsonArray() const;
    void fromJsonArray(const QJsonArray &array);
};

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(const QString &inputFile = QString(),
                            const bool isQuickMode = false,
                            QObject *parent = nullptr);

    void setQtDir(const QString &qtDir);
    QString qtDir() const;

    void loadCache();

signals:
    void finished() const;

public slots:
    void start();
    void clean();

protected slots:
    void saveCache() const;
    bool isFileDirty(const QString &file, const bool isQuickMode);

    void onParsed(const QString &file, const QString &source,
                  const QByteArray &checksum,
                  const QDateTime &modified,
                  const QDateTime &created);
    void onParseRequest(const QString &file);
    bool onRunMoc(const QString &file);
    // ibs commands
    void onTargetName(const QString &target);
    void onTargetType(const QString &type);
    void onQtModules(const QStringList &modules);
    void onDefines(const QStringList &defines);
    void onIncludes(const QStringList &includes);
    void onLibs(const QStringList &libs);
    void onRunTool(const QString &tool, const QStringList &args);

protected:
    bool compile(const QString &file);
    bool link() const;
    void parseFile(const QString &file);

private:
    void updateQtModules(const QStringList &modules);
    bool initializeMoc();
    bool runProcess(const QString &app, const QStringList &arguments) const;
    QString capitalizeFirstLetter(const QString &string) const;
    QString findFile(const QString &file, const QStringList &includeDirs) const;
    QStringList jsonArrayToStringList(const QJsonArray &array) const;

    const bool mQuickMode = false;
    bool mCacheEnabled = false;

    QString mInputFile;
    QString mQtDir;
    QStringList mQtModules;
    bool mQtIsMocInitialized = false;
    QStringList mQtIncludes;
    QStringList mQtLibs;
    QStringList mQtDefines;

    QStringList mCustomDefines;
    QStringList mCustomDefineFlags;
    QStringList mCustomIncludes;
    QStringList mCustomIncludeFlags;
    QStringList mCustomLibs;

    QString mTargetName = "default";
    QString mTargetType = Tags::targetApp;
    QString mTargetLibType = Tags::targetLibDynamic;
    QHash<QString, FileInfo> mParsedFiles;
    QStringList mObjectFiles;
    QStringList mMocFiles;
    QStringList mQrcFiles;
};
