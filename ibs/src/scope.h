#pragma once

#include <QByteArray>
#include <QString>
#include <QHash>

#include <QJsonObject>

#include "flags.h"
#include "fileinfo.h"

class Scope
{
public:
    explicit Scope(const QString &name, const QString &relativePath);

    QString name() const;
    QByteArray id() const;

    QJsonObject toJson() const;
    static Scope fromJson(const QJsonObject &json);

    QList<FileInfo> parsedFiles() const;
    void insertParsedFile(const FileInfo &fileInfo);
    FileInfo parsedFile(const QString &path) const;
    bool isParsed(const QString &path) const;

    void addIncludePaths(const QStringList &includes);
    QStringList customIncludeFlags() const;
    void autoScanForIncludes();

    QString findFile(const QString &file) const;

protected:
    Scope();
    QString findFile(const QString &file, const QStringList &includeDirs) const;

    const QString mRelativePath;
    const QString mName;
    const QByteArray mId;

    QStringList mCustomIncludes;
    QStringList mCustomIncludeFlags;
    QHash<QString, FileInfo> mParsedFiles;
};
