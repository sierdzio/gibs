#include <QCoreApplication>
#include <QCommandLineParser>

#include <QString>
#include <QDir>
#include <QDirIterator>

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

/*!
 * The purpose of this app is to extract Qt header information and store it
 * in a simple JSON object, so that gibs can use it later to auto-detect Qt
 * modules.
 */

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("");
    app.setOrganizationDomain("sierdzio.com");
    app.setApplicationName("qtheadermapper");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption("qt-dir", "Qr installation directory", "Qt directory path"));
    parser.process(app);

    const QString qtPath(parser.value("qt-dir"));
    const QString qtIncludePath(qtPath + "/include");

    if (!QDir(qtIncludePath).exists()) {
        qInfo() << "Qt include path does not exist!" << qtIncludePath;
        return 1;
    }

    QHash<QString, QString> result;
    QDirIterator it(qtIncludePath,
                    QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        // TODO: fill the hash. Or even better - a JSON object/ array.
        //result.append(it.next());
    }

    return app.exec();
}
