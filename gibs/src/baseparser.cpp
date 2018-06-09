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
    const QString command(commandString.startsWith(" ")? commandString : " " + commandString);

    // TODO: add error handling
    // TODO: handle spaces in target name
    /* TODO: optimize! Ideas:
     * make tag() into constexpr or inline
     * scan first word instead of repeating contains() so much
     */

    if(command.contains(tag(Tags::targetCommand))) {
        if (command.contains(tag(Tags::targetName))) {
            const QString arg(extractArguments(command, Tags::targetName));
            qDebug() << "Target name:" << arg;
            mScope->setTargetName(arg);
            emit targetName(arg);
        }

        if (command.contains(tag(Tags::targetType))) {
            const QStringList args(extractArguments(command, Tags::targetType).split(" "));

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

    if (command.contains(tag(Tags::qtModules))) {
        const QStringList args(extractArguments(command, Tags::qtModules).split(" "));
        qDebug() << "Enabling Qt modules:" << args;
        mScope->setQtModules(args);
        emit qtModules(args);
    }

    if (command.contains(tag(Tags::defines))) {
        const QStringList args(extractArguments(command, Tags::defines).split(" "));
        qDebug() << "Adding defines:" << args;
        mScope->addDefines(args);
        emit defines(args);
    }

    if (command.contains(tag(Tags::include))
            || command.contains(tag(Tags::includes))) {
        // Make sure we select proper tag
        const QLatin1String tag = command.contains(Tags::includes)?
                    Tags::includes : Tags::include;
        const QStringList args(extractArguments(command, tag)
                               .split(" ", QString::SkipEmptyParts));
        qDebug() << "Adding includes:" << args;
        mScope->addIncludePaths(args);
        emit includes(args);
    }

    if (command.contains(tag(Tags::libs)) and
            !command.contains(tag(Tags::targetCommand))) {
        const QStringList args(extractArguments(command, Tags::libs).split(" "));
        qDebug() << "Adding libs:" << args;
        mScope->addLibs(args);
        emit libs(args);
    }

    if (command.contains(tag(Tags::tool))) {
        const QStringList args(extractArguments(command, Tags::tool).split(" "));
        qDebug() << "Running tool:" << args;
        if (args.size() > 0) {
            // TODO: that will modify the scope, but our local mScope will be "old"
            // We need to synchronise them or switch to pointers (yep).
            emit runTool(args.at(0), args.mid(1));
        }
    }

    if (command.contains(tag(Tags::subproject))
            || command.contains(tag(Tags::subprojects)))
    {
        // Make sure we select proper tag
        const QLatin1String tag = command.contains(Tags::subprojects)? Tags::subprojects : Tags::subproject;
        const QStringList args(extractArguments(command, tag).split(" "));
        qDebug() << "Subprojects(s):" << args;
        if (args.size() > 0) {
            for (const auto &arg : qAsConst(args)) {
                emit subproject(mScope->id(), arg);
            }
        }
    }

    if (command.contains(tag(Tags::version))) {
        const QString arg(extractArguments(command, Tags::version));
        const QVersionNumber ver(QVersionNumber::fromString(arg));
        qDebug() << "Setting project version:" << ver.toString();
        mScope->setVersion(ver);
        emit version(ver);
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
