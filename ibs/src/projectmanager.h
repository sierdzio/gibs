#pragma once

#include <QObject>

#include "tags.h"

class QJsonArray;

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(const QString &inputFile = QString(), QObject *parent = nullptr);

    void setQtDir(const QString &qtDir);
    void loadCache();

signals:
    void finished() const;

public slots:
    void start();
    void clean();

protected slots:
    void saveCache() const;

    void onParsed(const QString &file, const QString &source);
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

private:
    void updateQtModules(const QStringList &modules);
    bool initializeMoc();
    bool runProcess(const QString &app, const QStringList &arguments) const;
    QString capitalizeFirstLetter(const QString &string) const;
    QString findFile(const QString &file, const QStringList &includeDirs) const;
    QStringList jsonArrayToStringList(const QJsonArray &array) const;

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
    QStringList mParsedFiles;
    QStringList mObjectFiles;
    QStringList mMocFiles;
    QStringList mQrcFiles;
};
