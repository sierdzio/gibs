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
    connect(this, &BaseParser::feature, scope, &Scope::onFeature);
}

bool BaseParser::parseCommand(const QString &commandString)
{
    // TODO: add error handling
    // TODO: handle spaces in target name
    /* TODO: optimize! Ideas:
     * make tag() into constexpr or inline
     * scan first word instead of repeating contains() so much
     */

    const QStringList allArguments(commandString.split(" ",
                                                       QString::SkipEmptyParts));

    if (allArguments.size() < 2) return true;

    const bool startsWithComment = (allArguments.at(0) == Tags::scopeOneLine
                                    or allArguments.at(0) == Tags::scopeBegin)?
                                    true : false;
    const QString &command(startsWithComment?
        allArguments.at(1) : allArguments.at(0));
    const QStringList arguments(startsWithComment?
        QStringList(allArguments.mid(1)) : allArguments);

    qDebug() << "Parsing command:" << commandString << "command is:" << command << "arguments:" << arguments;
    if(command == Tags::targetCommand) {
        if (arguments.at(1) == Tags::targetName) {
            const QString &arg(arguments.at(2));
            qDebug() << "Target name:" << arg;
            mScope->setTargetName(arg);
            emit targetName(arg);
        }

        if (arguments.at(1) == Tags::targetType) {
            const QStringList args(arguments.mid(2));

            // TODO: optimize!
            if (args.at(0) == Tags::targetApp or args.at(0) == Tags::targetLib) {
                qDebug() << "Target type:" << args;
                mScope->setTargetType(args.at(0));
                emit targetType(args.at(0));
                if (args.length() > 1 and (args.at(1) == Tags::targetLibDynamic
                                           or args.at(1) == Tags::targetLibStatic))
                {
                    mScope->setTargetLibType(args.at(1));
                    emit targetLibType(args.at(1));
                }
            } else {
                qFatal("Invalid target type: %s", qPrintable(args.join(" ")));
            }
        }
    } else if (command == Tags::qtModules) {
        const QStringList args(arguments.mid(1));
        qDebug() << "Enabling Qt modules:" << args;
        mScope->setQtModules(args);
        emit qtModules(args);
    } else if (command == Tags::defines) {
        const QStringList args(arguments.mid(1));
        qDebug() << "Adding defines:" << args;
        mScope->addDefines(args);
        emit defines(args);
    } else if (command == Tags::include || command == Tags::includes) {
        const QStringList args(arguments.mid(1));
        qDebug() << "Adding includes:" << args;
        mScope->addIncludePaths(args);
        emit includes(args);
    } else if (command == Tags::libs and command != Tags::targetCommand) {
        const QStringList args(arguments.mid(1));
        qDebug() << "Adding libs:" << args;
        mScope->addLibs(args);
        emit libs(args);
    } else if (command == Tags::tool) {
        const QStringList args(arguments.mid(1));
        qDebug() << "Running tool:" << args;
        if (args.size() > 0) {
            // TODO: that will modify the scope, but our local mScope will be "old"
            emit runTool(args.at(0), args.mid(1));
        }
    } else if (command == Tags::subproject or command == Tags::subprojects) {
        const QStringList args(arguments.mid(1));
        qDebug() << "Subprojects(s):" << args;
        if (args.size() > 0) {
            for (const auto &arg : args) {
                emit subproject(mScope->id(), arg);
            }
        }
    } else if (command == Tags::feature) {
        const QStringList args(arguments.mid(1));
        qDebug() << "Adding feature:" << args;
        if (args.size() > 0) {
            bool defaultOn = args.length() > 2? (args.at(2) == Tags::featureOn? true : false) : true;
            emit feature(args.at(0), defaultOn);
        }
    } else if (command == Tags::version) {
        const QString &arg(arguments.at(1));
        const QVersionNumber ver(QVersionNumber::fromString(arg));
        qDebug() << "Setting project version:" << ver.toString();
        mScope->setVersion(ver);
        emit version(ver);
    }

    return true;
}

QString BaseParser::extractArguments(const QString &line,
                                     const QLatin1String &tag) const
{
    return line.mid(line.indexOf(tag) + tag.size() + 1);
}
