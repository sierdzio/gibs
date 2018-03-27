#include "baseparser.h"
#include "tags.h"

#include <QDebug>

BaseParser::BaseParser(const Scope &scope, QObject *parent) : QObject(parent),
    mScope(scope)
{
}

bool BaseParser::parseCommand(const QString &commandString)
{
    // TODO: add error handling
    // TODO: handle spaces in target name

    if(commandString.contains(Tags::targetCommand)) {
        if (commandString.contains(Tags::targetName)) {
            const QString arg(extractArguments(commandString, Tags::targetName));
            qDebug() << "Target name:" << arg;
            mScope.setTargetName(arg);
            emit targetName(arg);
        }

        if (commandString.contains(Tags::targetType)) {
            const QString arg(extractArguments(commandString, Tags::targetType));

            if (arg == Tags::targetApp || arg == Tags::targetLib) {
                qDebug() << "Target type:" << arg;
                mScope.setTargetType(arg);
                emit targetType(arg);
            } else {
                qFatal("Invalid target type: %s", qPrintable(arg));
            }
        }
    }

    if (commandString.contains(Tags::qtModules)) {
        const QStringList args(extractArguments(commandString, Tags::qtModules).split(" "));
        qDebug() << "Enabling Qt modules:" << args;
        mScope.setQtModules(args);
        emit qtModules(args);
    }

    if (commandString.contains(Tags::defines)) {
        const QStringList args(extractArguments(commandString, Tags::defines).split(" "));
        qDebug() << "Adding defines:" << args;
        mScope.addDefines(args);
        emit defines(args);
    }

    if (commandString.contains(Tags::includes)) {
        const QStringList args(extractArguments(commandString, Tags::includes)
                               .split(" ", QString::SkipEmptyParts));
        qDebug() << "Adding includes:" << args;
        mScope.addIncludePaths(args);
        emit includes(args);
    }

    if (commandString.contains(Tags::libs)) {
        const QStringList args(extractArguments(commandString, Tags::libs).split(" "));
        qDebug() << "Adding libs:" << args;
        mScope.addLibs(args);
        emit libs(args);
    }

    if (commandString.contains(Tags::tool)) {
        const QStringList args(extractArguments(commandString, Tags::tool).split(" "));
        qDebug() << "Running tool:" << args;
        if (args.size() > 0) {
            // TODO: that will modify the scope, but our local mScope will be "old"
            // We need to synchronise them or switch to pointers (yep).
            emit runTool(args.at(0), args.mid(1));
        }
    }

    return true;
}

QString BaseParser::extractArguments(const QString &line, const QLatin1String &tag) const
{
    return line.mid(line.indexOf(tag) + tag.size() + 1);
}
