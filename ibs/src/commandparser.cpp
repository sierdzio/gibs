#include "commandparser.h"
#include "tags.h"

CommandParser::CommandParser(const QString &commands, QObject *parent)
    : BaseParser(parent),
      mCommandsRaw(commands)
{    
}

bool CommandParser::parse() const
{
    const QStringList commands(mCommandsRaw.split(Tags::commandSeparator));

    for (const QString &command : commands) {
        parseCommand(command);
    }

    return true;
}
