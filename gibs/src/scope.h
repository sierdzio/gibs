#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QHash>
#include <QScopedPointer>
#include <QVersionNumber>

#include <QJsonObject>
#include <QJsonArray>

#include "fileinfo.h"
#include "tags.h"
#include "metaprocess.h"
#include "gibs.h"
#include "flags.h"
#include "compiler.h"

class Scope;

using ScopePtr = QSharedPointer<Scope>;

class Scope : public QObject
{
    Q_OBJECT

public:
    explicit Scope(const QString &name, const QString &relativePath,
                   const Flags &flags,
                   const QHash<QString, Gibs::Feature> &features,
                   QObject *parent = nullptr);

    QString name() const;
    QByteArray id() const;
    QString relativePath() const;

    QJsonObject toJson() const;
    static Scope *fromJson(const QJsonObject &json, const Flags &flags);

    void mergeWith(const ScopePtr &other);
    void dependOn(const ScopePtr &other);
    bool isFinished() const;

    QList<FileInfo> parsedFiles() const;
    void insertParsedFile(const FileInfo &fileInfo);
    FileInfo parsedFile(const QString &path) const;
    bool isParsed(const QString &path) const;

    QStringList includePaths() const;
    QStringList customIncludeFlags() const;
    void autoScanForIncludes();

    QString findFile(const QString &file) const;

    QStringList qtModules() const;
    QStringList customDefineFlags() const;
    QStringList qtDefines() const;
    QStringList qtIncludes() const;
    QStringList qtLibs() const;
    QString targetName() const;
    QString targetType() const;
    QString targetLibType() const;
    QStringList customLibs() const;
    QString qtDir() const;

    bool qtIsMocInitialized() const;
    void setQtIsMocInitialized(bool qtIsMocInitialized);

    void setTargetLibType(const QString &targetLibType);

    QVector<QByteArray> scopeDependencyIds() const;

    QVersionNumber version() const;
    void setVersion(const QVersionNumber &version);

    QHash<QString, Gibs::Feature> features() const;

public slots:
    void start(bool fromCache, bool isQuickMode);
    void clean();

    void addIncludePaths(const QStringList &includes);
    void setTargetName(const QString &target);
    void setTargetType(const QString &type);
    void setQtModules(const QStringList &modules);
    void addDefines(const QStringList &defines);
    void addLibs(const QStringList &libs);
    void onFeature(const QString &name, const bool isOn);

    void setCompiler(const Compiler &compiler);

signals:
    void error(const QString &error) const;
    void runProcess(const QString &app, const QStringList &arguments, const MetaProcessPtr &mp) const;
    void subproject(const QByteArray &scopeId, const QString &path) const;    
    void feature(const Gibs::Feature &feature) const;

protected:
    QString compile(const QString &file);
    void link();
    void deploy();
    void parseFile(const QString &file);
    bool isFileDirty(const QString &file, const bool isQuickMode) const;

protected slots:
    void onParsed(const QString &file, const QString &source,
                  const QByteArray &checksum,
                  const QDateTime &modified,
                  const QDateTime &created);
    void onParseRequest(const QString &file,
                        const bool force = false);
    bool onRunMoc(const QString &file);
    void onRunTool(const QString &tool,
                   const QStringList &args);

protected:
    Scope(const QByteArray &id, const QString &name, const QString &relativePath,
          const Flags &flags);
    QString findFile(const QString &file, const QStringList &includeDirs) const;
    bool isFromSubproject(const QString &file) const;
    void updateQtModules(const QStringList &modules);

    MetaProcessPtr findDependency(const QString &file) const;
    QVector<MetaProcessPtr> findDependencies(const QString &file) const;
    QVector<MetaProcessPtr> findAllDependencies() const;

    bool initializeMoc();

    Flags mFlags;
    Compiler mCompiler;

    const QString mRelativePath;
    const QString mName;
    QVersionNumber mVersion = QVersionNumber(1, 0, 0);
    const QByteArray mId;

    bool mIsError = false;

    QStringList mQtModules;
    bool mQtIsMocInitialized = false;
    QStringList mQtIncludes;
    QStringList mQtLibs;
    QStringList mQtDefines;

    QStringList mCustomDefines;
    QStringList mCustomDefineFlags;
    QStringList mCustomLibs;

    QString mTargetName = "default";
    QString mTargetType = Tags::targetApp;
    QString mTargetLibType = Tags::targetLibDynamic;

    QStringList mCustomIncludes;
    QStringList mCustomIncludeFlags;
    QHash<QString, FileInfo> mParsedFiles;
    // Name, Feature
    QHash<QString, Gibs::Feature> mFeatures;
    QVector<QByteArray> mScopeDependencyIds;
    QVector<ScopePtr> mScopeDependencies;
    // TODO: change into QStringList and use only file names here.
    // MetaProcessPtr can remain in ProjectManager, but not really here.
    QVector<MetaProcessPtr> mProcessQueue; // Local process queue
};
