#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonArray>

/*!
 * \brief The FileInfo class contains information about a single compilation
 * unit.
 *
 * That is, when the compiler compiles some .cpp file, it will generate an
 * object file. With Qt support on, some files will also have corresponding MOC
 * files etc. FileInfo keeps information about such corelated files in one place.
 *
 * \todo good candidate for refactoring, this class is.
 */
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
