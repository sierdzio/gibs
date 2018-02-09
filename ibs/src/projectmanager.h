#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

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
    void onTargetName(const QString &target);
    void onTargetType(const QString &type);
    void onQtModules(const QStringList &modules);
    void onIncludes(const QStringList &includes);
    void onLibs(const QStringList &libs);

protected:
    bool compile(const QString &file);
    bool link();

private:
    const QString mInputFile;
    QString mQtDir;
    QStringList mQtModules;
    QStringList mCustomIncludes;
    QStringList mCustomLibs;

    QString mTargetName = "default";
    QString mTargetType = Tags::targetApp;
    QString mTargetLibType = Tags::targetLibDynamic;
    QVector<QString> mParsedFiles;
    QVector<QString> mObjectFiles;
};
