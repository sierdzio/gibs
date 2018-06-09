#include "commandparser.h"
#include "tags.h"

CommandParser::CommandParser(const QString &commands, Scope *scope, QObject *parent)
    : BaseParser(scope, parent),
      mCommandsRaw(commands)
{    
}

bool CommandParser::parse()
{
    const QStringList commands(mCommandsRaw.split(Tags::commandSeparator));
    for (const QString &command : commands) {
        parseCommand(command);
    }

    return true;
}
