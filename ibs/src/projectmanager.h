#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(const QString &inputFile, QObject *parent = nullptr);

signals:
    void finished() const;

public slots:
    void start();

protected slots:
    void onParsed(const QString &file, const QString &source);
    void onParseRequest(const QString &file);

protected:
    bool compile(const QString &file);
    bool link();

private:
    QString mProjectName = "default";
    const QString mInputFile;
    QVector<QString> mParsedFiles;
    QVector<QString> mObjectFiles;
};
