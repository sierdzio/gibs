#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

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
    void onQtModules(const QStringList &modules);

protected:
    bool compile(const QString &file);
    bool link();

private:
    const QString mInputFile;
    QString mQtDir;
    QStringList mQtModules;

    QString mTargetName = "default";
    QVector<QString> mParsedFiles;
    QVector<QString> mObjectFiles;
};
