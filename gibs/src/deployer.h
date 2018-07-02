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
    QJsonObject toJson() const;
    static Deployer fromJson(const QJsonObject &json);
    static Deployer fromFile(const QString &jsonFile);
    static QString findDeployer(const QString &name);

    QString name = "linuxdeployqt";
    QString suffix = "AppImage";
    QStringList flags = {
        "-verbose=1",
        "-no-translations",
        "-no-copy-copyright-files",
        "-appimage"
    };
};
