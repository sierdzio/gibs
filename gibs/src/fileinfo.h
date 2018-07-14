#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonArray>

class FileInfo {

    Q_GADGET

public:
    enum FileType {
        Cpp,
        QRC
    }; Q_ENUM(FileType)

    QString path;
    QDateTime dateModified;
    QDateTime dateCreated;
    QByteArray checksum;
    // if --pipe flag is used, this will contain full file contents
    QByteArray contents;
    QString objectFile;
    QString generatedFile;
    QString generatedObjectFile;
    FileType type;

    bool isEmpty() const;
    QJsonArray toJsonArray() const;
    void fromJsonArray(const QJsonArray &array);

private:
    QString fileTypeToString(const FileType type) const;
    FileType stringToFileType(const QString &type) const;
};
