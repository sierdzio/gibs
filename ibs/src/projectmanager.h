#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(const QString &inputFile, QObject *parent = nullptr);

public slots:
    void start();

protected slots:
    void onParsed(const QString &file);

protected:
    bool compile(const QString &file);
    bool link();

private:
    const QString mInputFile;
    QVector<QString> mParsedFiles;
};
