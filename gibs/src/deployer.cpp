#include "deployer.h"
#include "tags.h"
#include "gibs.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QFile>
#include <QDir>

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

QString Deployer::findDeployer(const QString &name)
{
    return Gibs::findJsonToolDefinition(name, Gibs::Deployer);
}
