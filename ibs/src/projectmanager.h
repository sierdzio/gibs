#pragma once

#include <QObject>

#include "tags.h"

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(const QString &inputFile, QObject *parent = nullptr);

    void setQtDir(const QString &qtDir);

signals:
    void finished() const;

public slots:
    void start();

protected slots:
    void onParsed(const QString &file, const QString &source);
    void onParseRequest(const QString &file);
    bool onRunMoc(const QString &file);
    // ibs commands
    void onTargetName(const QString &target);
    void onTargetType(const QString &type);
    void onQtModules(const QStringList &modules);
    void onIncludes(const QStringList &includes);
    void onLibs(const QStringList &libs);

protected:
    bool compile(const QString &file);
    bool link() const;

private:
    void updateQtModules(const QStringList &modules);
    bool initializeMoc();
    bool runProcess(const QString &app, const QStringList &arguments) const;
    QString capitalizeFirstLetter(const QString &string) const;

    const QString mInputFile;
    QString mQtDir;
    QStringList mQtModules;
    bool mQtIsMocInitialized = false;
    QStringList mQtIncludes;
    QStringList mQtLibs;
    QStringList mQtDefines;

    QStringList mCustomIncludes;
    QStringList mCustomLibs;

    QString mTargetName = "default";
    QString mTargetType = Tags::targetApp;
    QString mTargetLibType = Tags::targetLibDynamic;
    QStringList mParsedFiles;
    QStringList mObjectFiles;
};
