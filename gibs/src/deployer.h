#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>

/*!
 * \brief The Deployer struct - contains all necessary info to run deployment
 * using given compiler.
 *
 * "Deployer" because it's shorter than "DeploymentTool".
 */
struct Deployer
{
    bool findAndSetExecutable(const QString &userProvided,
                              const QString &qtDir,
                              const QString &path);
    QJsonObject toJson() const;
    static Deployer fromJson(const QJsonObject &json);
    static Deployer fromFile(const QString &jsonFile);
    static QString find(const QString &name);

    QString executable;

    QString name = "linuxdeployqt";
    QString suffix = "AppImage";
    QStringList flags = {
        "-verbose=1",
        "-no-translations",
        "-no-copy-copyright-files",
        "-appimage"
    };
};
