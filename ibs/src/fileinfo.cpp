#include "fileinfo.h"

#include <QMetaObject>
#include <QMetaEnum>

bool FileInfo::isEmpty() const
{
    return (path.isEmpty() and checksum.isEmpty());
}

QJsonArray FileInfo::toJsonArray() const
{
    QJsonArray result;
    int i = 0;
    result.insert(i++, path);
    result.insert(i++, QString(checksum.toHex()));
    result.insert(i++, dateModified.toString(Qt::ISODateWithMs));
    result.insert(i++, dateCreated.toString(Qt::ISODateWithMs));
    result.insert(i++, objectFile);
    result.insert(i++, generatedFile);
    result.insert(i++, generatedObjectFile);
    result.insert(i++, fileTypeToString(type));
    return result;
}

void FileInfo::fromJsonArray(const QJsonArray &array)
{
    int i = 0;
    path = array.at(i++).toString();
    checksum = QByteArray::fromHex(array.at(i++).toString().toLatin1());
    dateModified = QDateTime::fromString(array.at(i++).toString(), Qt::ISODateWithMs);
    dateCreated = QDateTime::fromString(array.at(i++).toString(), Qt::ISODateWithMs);
    objectFile = array.at(i++).toString();
    generatedFile = array.at(i++).toString();
    generatedObjectFile = array.at(i++).toString();
    type = stringToFileType(array.at(i++).toString());
}

QString FileInfo::fileTypeToString(const FileInfo::FileType type) const
{
    const auto smo = FileInfo::staticMetaObject;
    const auto enumerator = smo.enumerator(smo.indexOfEnumerator("FileType"));
    return enumerator.valueToKey(type);
}

FileInfo::FileType FileInfo::stringToFileType(const QString &type) const
{
    const auto smo = FileInfo::staticMetaObject;
    const auto enumerator = smo.enumerator(smo.indexOfEnumerator("FileType"));
    return FileType(enumerator.keyToValue(type.toLatin1().constData()));
}
