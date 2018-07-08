#include "gibs.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>

#include <QDebug>

void Gibs::removeFile(const QString &path) {
    if (QFile::exists(path)) {
        qInfo() << "Removing:" << path;
        QFile::remove(path);
    }
}

QString Gibs::findFile(const QString &directory, const QString &name)
{
    const QDir dir(directory);
    const QStringList files(dir.entryList(
        QDir::Files | QDir::NoDotAndDotDot));
    const int index = files.indexOf(name);
    if (index != -1) {
        qDebug() << "Found file:" << dir.path() + "/" + name;
        return dir.path() + "/" + name;
    }
    return QString();
}

QJsonDocument Gibs::readJsonFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qFatal("Could not open file for reading. %s",
               qPrintable(path));
    }

    return QJsonDocument::fromJson(file.readAll());
}

/*!
 * For given feature \a name (example: "tts-support") returns an upper-cased
 * and sanitized name, suitable for a c++ define (example: "TTS_SUPPORT").
 */
QString Gibs::normalizeFeatureName(const QString &name)
{
    QString result(name.toUpper());
    result = result.replace('-', '_');
    return result;
}

/*!
 * Converts a \a command line flag into Feature and returns it.
 *
 * For example:
 * $ gibs main.cpp -- --tts-support --no-opengl
 *
 * If you call this function for boths flags, it will enable TTS support, but
 * keep OpenGL turned off.
 */
Gibs::Feature Gibs::commandLineToFeature(const QString &command)
{
    Gibs::Feature result;
    const QString negative("--no-");

    if (command.length() < 2 or !command.startsWith("--")) {
        qFatal("Invalid feature name: %s", qPrintable(command));
    }

    if (command.startsWith(negative)) {
        result.name = command.mid(negative.length());
        result.enabled = false;
    } else {
        // Cut '--' from beginning of the string:
        result.name = command.mid(2);
        result.enabled = true;
    }

    result.define = Gibs::normalizeFeatureName(result.name);
    // result.defined is intentionally left out. Scope is responsible for setting
    // it.

    return result;
}

QString Gibs::capitalizeFirstLetter(const QString &string)
{
    return (string[0].toUpper() + string.mid(1));
}

QStringList Gibs::jsonArrayToStringList(const QJsonArray &array)
{
    QStringList result;

    for (const auto &value : array) {
        result.append(value.toString());
    }

    return result;
}

// TODO: use QDir::setSearchPaths()?
QString Gibs::findJsonToolDefinition(const QString &tool,
                                     const Gibs::ToolType type)
{
    QString result;
    const QString fileName(tool + ".json");
    const QString toolDir(type == Gibs::Compiler? "compilers" : "deployers");
    // First, check $HOME/.gibs
    {
        QDir homeDir(QDir::homePath());
        homeDir.cd(".gibs");
        homeDir.cd(toolDir);
        if (homeDir.exists()) {
            result = Gibs::findFile(homeDir.path(), fileName);
        } else {
            qDebug() << "No tool found in" << homeDir.path();
        }

        if (!result.isEmpty())
            return result;
    }

    // Second, check internal database
    result = Gibs::findFile(":/" + toolDir, fileName);
    return result;
}
