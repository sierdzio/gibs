#include "deployer.h"
#include "tags.h"
#include "gibs.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QFile>
#include <QDir>

/*!
 * Looks for deployer executable and returns true if it is found.
 *
 * It will look for exe in the following order: first, the \a userProvided
 * path (should include executable), then inside \a qtDir. Lastly, system
 *  \a path will be searched.
 */
bool Deployer::findAndSetExecutable(const QString &path,
                              const QString &qtDir,
                              const QString &userProvided)
{
    if (executable.isEmpty() == false)
        return true;

    if (!userProvided.isEmpty()) {
        if (QFileInfo info(userProvided); info.exists() and info.isExecutable()) {
            executable = userProvided;
            return true;
        } else {
            qWarning() << "Deployer executable does not exist or is not executable"
                       << userProvided;
        }
    }

    if (!qtDir.isEmpty()) {
        const QString exec(qtDir + "/bin/" + name);
        if (QFileInfo info(exec); info.exists() and info.isExecutable()) {
            executable = exec;
            return true;
        }
    }

    if (!path.isEmpty()) {
        // TODO: implement path searching!
        qWarning() << "Cannot look for" << name << "in $PATH because this logic"
                   << "is not yet implemented! Sorry.";
    }

    return false;
}

QJsonObject Deployer::toJson() const
{
    QJsonObject object;
    object.insert(Tags::deployerName, name);
    object.insert(Tags::deployerSuffix, suffix);
    object.insert(Tags::deployerFlags, QJsonArray::fromStringList(flags));
    return object;
}

Deployer Deployer::fromJson(const QJsonObject &json)
{
    Deployer deployer;
    deployer.name = json.value(Tags::deployerName).toString();
    deployer.suffix = json.value(Tags::deployerSuffix).toString();
    deployer.flags = Gibs::jsonArrayToStringList(
                json.value(Tags::deployerFlags).toArray());
    return deployer;
}

Deployer Deployer::fromFile(const QString &jsonFile)
{    
    qDebug() << "Loading deployer:" << jsonFile;
    return fromJson(Gibs::readJsonFile(jsonFile).object());
}

QString Deployer::find(const QString &name)
{
    return Gibs::findJsonToolDefinition(name, Gibs::Deployer);
}
