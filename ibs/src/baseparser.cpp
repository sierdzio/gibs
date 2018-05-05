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

    if(commandString.contains(tag(Tags::targetCommand))) {
        if (commandString.contains(tag(Tags::targetName))) {
            const QString arg(extractArguments(commandString, Tags::targetName));
            qDebug() << "Target name:" << arg;
            mScope->setTargetName(arg);
            emit targetName(arg);
        }

        if (commandString.contains(tag(Tags::targetType))) {
            const QStringList args(extractArguments(commandString, Tags::targetType).split(" "));

            // TODO: optimize!
            if (args.at(0) == Tags::targetApp or args.at(0) == Tags::targetLib) {
                qDebug() << "Target type:" << args;
                mScope->setTargetType(args.at(0));
                emit targetType(args.at(0));
                if (args.length() > 1 and (args.at(1) == Tags::targetLibDynamic
                                           or args.at(1) == Tags::targetLibStatic)) {
                    mScope->setTargetLibType(args.at(1));
                    emit targetLibType(args.at(1));
                }
            } else {
                qFatal("Invalid target type: %s", qPrintable(args.join(" ")));
            }
        }
    }

    if (commandString.contains(tag(Tags::qtModules))) {
        const QStringList args(extractArguments(commandString, Tags::qtModules).split(" "));
        qDebug() << "Enabling Qt modules:" << args;
        mScope->setQtModules(args);
        emit qtModules(args);
    }

    if (commandString.contains(tag(Tags::defines))) {
        const QStringList args(extractArguments(commandString, Tags::defines).split(" "));
        qDebug() << "Adding defines:" << args;
        mScope->addDefines(args);
        emit defines(args);
    }

    if (commandString.contains(tag(Tags::includes))) {
        const QStringList args(extractArguments(commandString, Tags::includes)
                               .split(" ", QString::SkipEmptyParts));
        qDebug() << "Adding includes:" << args;
        mScope->addIncludePaths(args);
        emit includes(args);
    }

    if (commandString.contains(tag(Tags::libs)) and
            !commandString.contains(tag(Tags::targetCommand))) {
        const QStringList args(extractArguments(commandString, Tags::libs).split(" "));
        qDebug() << "Adding libs:" << args;
        mScope->addLibs(args);
        emit libs(args);
    }

    if (commandString.contains(tag(Tags::tool))) {
        const QStringList args(extractArguments(commandString, Tags::tool).split(" "));
        qDebug() << "Running tool:" << args;
        if (args.size() > 0) {
            // TODO: that will modify the scope, but our local mScope will be "old"
            // We need to synchronise them or switch to pointers (yep).
            emit runTool(args.at(0), args.mid(1));
        }
    }

    if (commandString.contains(tag(Tags::subproject))
            || commandString.contains(tag(Tags::subprojects)))
    {
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

QString BaseParser::tag(const QLatin1String &rawTag) const
{
    return QString(" " + rawTag + " ");
}
