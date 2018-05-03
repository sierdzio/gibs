#include "baseparser.h"
#include "tags.h"

#include <QDebug>

BaseParser::BaseParser(Scope *scope, QObject *parent) : QObject(parent),
    mScope(scope)
{    
    connect(this, &BaseParser::targetName, scope, &Scope::setTargetName);
    connect(this, &BaseParser::targetType, scope, &Scope::setTargetType);
    connect(this, &BaseParser::qtModules, scope, &Scope::setQtModules);
    connect(this, &BaseParser::defines, scope, &Scope::addDefines);
    connect(this, &BaseParser::includes, scope, &Scope::addIncludePaths);
    connect(this, &BaseParser::libs, scope, &Scope::addLibs);
}

bool BaseParser::parseCommand(const QString &commandString)
{
    // TODO: add error handling
    // TODO: handle spaces in target name

    if(commandString.contains(Tags::targetCommand)) {
        if (commandString.contains(Tags::targetName)) {
            const QString arg(extractArguments(commandString, Tags::targetName));
            qDebug() << "Target name:" << arg;
            mScope->setTargetName(arg);
            emit targetName(arg);
        }

        if (commandString.contains(Tags::targetType)) {
            const QString arg(extractArguments(commandString, Tags::targetType));

            if (arg == Tags::targetApp || arg == Tags::targetLib) {
                qDebug() << "Target type:" << arg;
                mScope->setTargetType(arg);
                emit targetType(arg);
            } else {
                qFatal("Invalid target type: %s", qPrintable(arg));
            }
        }
    }

    if (commandString.contains(Tags::qtModules)) {
        const QStringList args(extractArguments(commandString, Tags::qtModules).split(" "));
        qDebug() << "Enabling Qt modules:" << args;
        mScope->setQtModules(args);
        emit qtModules(args);
    }

    if (commandString.contains(Tags::defines)) {
        const QStringList args(extractArguments(commandString, Tags::defines).split(" "));
        qDebug() << "Adding defines:" << args;
        mScope->addDefines(args);
        emit defines(args);
    }

    if (commandString.contains(Tags::includes)) {
        const QStringList args(extractArguments(commandString, Tags::includes)
                               .split(" ", QString::SkipEmptyParts));
        qDebug() << "Adding includes:" << args;
        mScope->addIncludePaths(args);
        emit includes(args);
    }

    if (commandString.contains(Tags::libs)) {
        const QStringList args(extractArguments(commandString, Tags::libs).split(" "));
        qDebug() << "Adding libs:" << args;
        mScope->addLibs(args);
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

    if (commandString.contains(Tags::subproject)) {
        // Make sure we select proper tag
        const QLatin1String tag = commandString.contains(Tags::subprojects)? Tags::subprojects : Tags::subproject;
        const QStringList args(extractArguments(commandString, tag).split(" "));
        qDebug() << "Subprojects(s):" << args;
        if (args.size() > 0) {
            for (const auto &arg : qAsConst(args)) {
                emit subproject(mScope->id(), arg);
            }
        }
    }

    return true;
}

QString BaseParser::extractArguments(const QString &line, const QLatin1String &tag) const
{
    return line.mid(line.indexOf(tag) + tag.size() + 1);
}
