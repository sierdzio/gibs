#include "scope.h"
#include "tags.h"

#include <QDirIterator>
#include <QCryptographicHash>
#include <QJsonArray>

#include <QDebug>

Scope::Scope(const QString &name, const QString &relativePath)
    : mRelativePath(relativePath),
      mName(name),
      mId(QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha1))
{
    addIncludePaths({"."});
}

QString Scope::name() const
{
    return mName;
}

QByteArray Scope::id() const
{
    return mId;
}

QJsonObject Scope::toJson() const
{
    QJsonObject object;
    QJsonArray filesArray;
    for (const auto &file : mParsedFiles.values()) {
        if (!file.isEmpty())
            filesArray.append(file.toJsonArray());
    }

    object.insert(Tags::parsedFiles, filesArray);
    // TODO: implement
    return object;
}

Scope Scope::fromJson(const QJsonObject &json)
{
    Scope scope;
    const QJsonArray filesArray = json.value(Tags::parsedFiles).toArray();
    for (const auto &file : filesArray) {
        FileInfo fileInfo;
        fileInfo.fromJsonArray(file.toArray());
        scope.mParsedFiles.insert(fileInfo.path, fileInfo);
    }

    // TODO: implement
    return scope;
}

QList<FileInfo> Scope::parsedFiles() const
{
    return mParsedFiles.values();
}

void Scope::insertParsedFile(const FileInfo &fileInfo)
{
    mParsedFiles.insert(fileInfo.path, fileInfo);
}

FileInfo Scope::parsedFile(const QString &path) const
{
    return mParsedFiles.value(path);
}

bool Scope::isParsed(const QString &path) const
{
    return mParsedFiles.contains(path);
}

void Scope::addIncludePaths(const QStringList &includes)
{
    mCustomIncludes += includes;
    mCustomIncludes.removeDuplicates();
    for(const auto &incl : qAsConst(mCustomIncludes)) {
        const QString &inc("-I"+ mRelativePath + "/" + incl);
        if (!mCustomIncludeFlags.contains(inc)) {
            mCustomIncludeFlags.append(inc);
        }
    }
    qInfo() << "Updating custom includes:" << mCustomIncludes;
}

QStringList Scope::customIncludeFlags() const
{
    return mCustomIncludeFlags;
}

/*!
 * Scans \a path for all subdirectories and adds them to include dir list.
 *
 * This can be used to automatically guess include paths in a project.
 *
 * \a path can be a file - the function will search for all subdirs of the
 * directory where that file is located.
 */
void Scope::autoScanForIncludes()
{
    QStringList result;

    // Make sure we don't deal with a file
    QDirIterator it(QFileInfo(mName).path(),
                    QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        result.append(it.next());
    }

    addIncludePaths(result);
}

QString Scope::findFile(const QString &file) const
{
    return findFile(file, mCustomIncludes);
}

// Protected constructor - used in fromJson().
Scope::Scope()
{
}

QString Scope::findFile(const QString &file, const QStringList &includeDirs) const
{
    QString result;
    if (file.contains(mRelativePath))
        result = file;
    else
        result = mRelativePath + "/" + file;

    // Search through include paths
    if (QFileInfo(result).exists()) {
        //qDebug() << "RETURNING:" << result;
        return result;
    }

    for (const QString &inc : qAsConst(includeDirs)) {
        const QString tempResult(mRelativePath + "/" + inc + "/" + file);
        if (QFileInfo(tempResult).exists()) {
            result = tempResult;
            break;
        }
    }

    //qDebug() << "FOUND:" << result;
    return result;
}