/*******************************************************************************
Copyright (C) 2017 Milo Solutions
Contact: https://www.milosolutions.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/


/*
  TEMPLATE main.cpp by Milo Solutions. Copyright 2016
*/

/*i
 target name ibs
 qt core
 */

#include <QCoreApplication>
#include <QLoggingCategory>

#include <QString>
#include <QCommandLineParser>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>

#include "globals.h"
#include "tags.h"
#include "flags.h"
#include "projectmanager.h"

// Prepare logging categories. Modify these to your needs
//Q_DECLARE_LOGGING_CATEGORY(core) // already declared in MLog header
Q_LOGGING_CATEGORY(coreMain, "core.main")

/*!
  Main routine. Remember to update the application name and initialise logger
  class, if present.
  */
int main(int argc, char *argv[]) {
    //MiloLog::instance();
    // Set up basic application data. Modify this to your needs
    QCoreApplication app(argc, argv);

    QElapsedTimer timer;
    timer.start();

    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("");
    app.setOrganizationDomain("sierdzio.com");
    app.setApplicationName("ibs");
    //logger()->enableLogToFile(app.applicationName());
    qCInfo(coreMain) << "\n\tName:" << app.applicationName()
                     << "\n\tOrganisation:" << app.organizationName()
                     << "\n\tDomain:" << app.organizationDomain()
                     << "\n\tVersion:" << app.applicationVersion()
                     << "\n\tSHA:" << GIT_COMMIT_ID;


    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    const char *scope = "main";
    parser.addPositionalArgument(Tags::inputFile, QCoreApplication::translate(scope, "Input file, usually main.cpp"), "[input]");

    parser.addOptions({
        {{"r", Tags::run},
        QCoreApplication::translate(scope, "Run the executable immediately after building")},
        {Tags::qt_dir_flag,
        QCoreApplication::translate(scope, "Specify Qt directory for Qt apps"),
        QCoreApplication::translate(scope, "Qt dir")},
        {Tags::clean,
        QCoreApplication::translate(scope, "Clear build directory")},
        {{"q", Tags::quick_flag},
        QCoreApplication::translate(scope, "'Convention over configuration' mode - parse files only up to first line of 'concrete code'. Do not check file checksums when doing incremental builds.")},
        {{"j", Tags::jobs},
        QCoreApplication::translate(scope, "Max number of threads used to compile and process the sources. If not specified, ibs will use max possible number of threads"),
        QCoreApplication::translate(scope, "Thread count"),
        "0"},
        {{"c", Tags::commands},
        QCoreApplication::translate(scope, "ibs syntax commands - same you can specify in c++ commends. All commands are suppored on the command line as well"),
        QCoreApplication::translate(scope, "ibs commands, separated by semicolon"),
        ""}
    });

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty())
        qFatal("Input file is not set. Cannot continue. Please set the input file to your main c++ file");

    qDebug() << "Arguments:" << args;

    bool jobsOk = false;
    Flags flags;
    flags.setRun(parser.isSet(Tags::run));
    flags.setClean(parser.isSet(Tags::clean));
    flags.setQuickMode(parser.isSet(Tags::quick_flag));
    flags.setJobs(parser.value(Tags::jobs).toInt(&jobsOk));
    flags.setQtDir(parser.value(Tags::qt_dir_flag));
    flags.setInputFile(args.at(0));
    flags.setCommands(parser.value(Tags::commands));

    if (!jobsOk) {
        qFatal("Invalid number of jobs specified. Use '-j NUM'. Got: %s",
               qPrintable(parser.value(Tags::jobs)));
    } else {
        // TODO: use Flags::toString to nicely print all flags!
        qInfo() << "Maximum number of jobs:" << flags.jobs();
    }

    if (flags.clean()) {
        ProjectManager manager(flags);
        manager.loadCache();
        QObject::connect(&manager, &ProjectManager::finished, &app, &QCoreApplication::quit);
        QTimer::singleShot(1, &manager, &ProjectManager::clean);
        int result = app.exec();
        qInfo() << "Cleaning took:" << timer.elapsed() << "ms";
        return result;
    } else {
        ProjectManager manager(flags);
        manager.loadCache();
        if (!flags.qtDir().isEmpty() && (flags.qtDir() != manager.qtDir()))
            manager.setQtDir(flags.qtDir());
        QObject::connect(&manager, &ProjectManager::finished, &app, &QCoreApplication::quit);
        QTimer::singleShot(1, &manager, &ProjectManager::start);

        if (flags.run()) {
            qInfo() << "Running compiled binary";
        }

        int result = app.exec();
        qInfo() << "Build took:" << timer.elapsed() << "ms";
        return result;
    }

    return 1;
}
