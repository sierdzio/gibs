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
 target name gibs
 qt core
 */

#include <QCoreApplication>
#include <QLoggingCategory>

#include <QString>
#include <QCommandLineParser>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>

#include "mlog.h"

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
    MLog::instance();
    // Set up basic application data. Modify this to your needs
    QCoreApplication app(argc, argv);

    QElapsedTimer timer;
    timer.start();

    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("");
    app.setOrganizationDomain("sierdzio.com");
    app.setApplicationName("gibs");
    //logger()->enableLogToFile(app.applicationName());
    qCInfo(coreMain) << "\n\tName:" << app.applicationName()
                     << "\n\tOrganisation:" << app.organizationName()
                     << "\n\tDomain:" << app.organizationDomain()
                     << "\n\tVersion:" << app.applicationVersion()
                     << "\n\tSHA:" << GIT_COMMIT_ID
                     << "\n\tArgs:" << app.arguments();


    QCommandLineParser parser;
    parser.setApplicationDescription("C++ in-source project builder. Compile your "
                                     "projects without all the hassle connected "
                                     "with preparing a project file. Just run "
                                     "'gibs main.cpp' and enjoy your compiled "
                                     "binary! More info: "
                                     "https://github.com/sierdzio/ibs");
    parser.addHelpOption();
    parser.addVersionOption();

    const char *scope = "main";
    parser.addPositionalArgument(Tags::inputFile, QCoreApplication::translate(scope, "Input file, usually main.cpp"), "[input]");

    parser.addOptions({
        {{"d", Tags::debug},
        QCoreApplication::translate(scope, "Compile in debug mode. By default, gibs compiles release binaries")},
        {{"r", Tags::run},
        QCoreApplication::translate(scope, "Run the executable immediately after building")},
        {Tags::qt_dir_flag,
        QCoreApplication::translate(scope, "Specify Qt directory for Qt apps"),
        QCoreApplication::translate(scope, "Qt dir")},
        {{"m", Tags::auto_qt_modules_flag},
        QCoreApplication::translate(scope, "Automatically guess Qt modules used by the project. This is done using internal dictionary mapping Qt classes to modules.")},
        {Tags::clean,
        QCoreApplication::translate(scope, "Clear build directory")},
        {{"q", Tags::quick_flag},
        QCoreApplication::translate(scope, "'Convention over configuration' mode - parse files only up to first line of 'concrete code'. Do not check file checksums when doing incremental builds.")},
        {{"a", Tags::auto_include_flag},
        QCoreApplication::translate(scope, "Automatically scan source directory for include paths. This can be used instead of gibs command 'include some/path' if the path is below input file.")},
        {{"j", Tags::jobs},
        QCoreApplication::translate(scope, "Max number of threads used to compile and process the sources. If not specified, gibs will use max possible number of threads. If a fraction is specified, it will use given percentage of available cores (-j 0.5 means half of all CPU cores)"),
        QCoreApplication::translate(scope, "threads"),
        "0"},
        {{"c", Tags::commands},
        QCoreApplication::translate(scope, "gibs syntax commands - same you can specify in c++ commends. All commands are suppored on the command line as well"),
        QCoreApplication::translate(scope, "commands"),
        ""},
        {{"w", Tags::parse_whole_files},
        QCoreApplication::translate(scope, "Parse whole files instead of just their beginning. By default, only code up to first class declaration or function definition is parsed.")},
        {Tags::deploy_tool,
        QCoreApplication::translate(scope, "path to deployment tool to use, for example linuxdeployqt.AppImage"),
        QCoreApplication::translate(scope, "path")},
    });

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    bool jobsOk = false;

    Flags flags;
    flags.setDebugBuild(parser.isSet(Tags::debug));
    // TODO: support debug-and-release maybe.
    if (flags.debugBuild())
        flags.setReleaseBuild(false);
    flags.setRun(parser.isSet(Tags::run));
    flags.setClean(parser.isSet(Tags::clean));
    flags.setQuickMode(parser.isSet(Tags::quick_flag));
    flags.setQtAutoModules(parser.isSet(Tags::auto_qt_modules_flag));
    flags.setAutoIncludes(parser.isSet(Tags::auto_include_flag));
    flags.setParseWholeFiles(parser.isSet(Tags::parse_whole_files));
    flags.setJobs(parser.value(Tags::jobs).toFloat(&jobsOk));
    flags.setQtDir(parser.value(Tags::qt_dir_flag));
    if (!args.isEmpty())
        flags.setInputFile(args.at(0));
    flags.setCommands(parser.value(Tags::commands));
    flags.setRelativePath(flags.inputFile());
    flags.setDeployTool(parser.value(Tags::deploy_tool));

    if (!jobsOk) {
        qFatal("Invalid number of jobs specified. Use '-j NUM'. Got: %s",
               qPrintable(parser.value(Tags::jobs)));
    } else {
        // TODO: use Flags::toString to nicely print all flags!
        qInfo() << "Maximum number of jobs:" << flags.jobs();
    }

    // Extract features:
    QHash<QString, Gibs::Feature> features;
    if (args.length() > 1) {
        const auto commands = args.mid(1);
        for (const auto &command: commands) {
            const auto &feature = Gibs::commandLineToFeature(command);
            features.insert(feature.name, feature);
            qDebug() << "Feature:" << feature.name
                     << "enabled:" << feature.enabled;
        }
    }

    ProjectManager manager(flags);
    manager.loadCache();
    manager.loadCommands();
    manager.loadFeatures(features);
    QObject::connect(&manager, &ProjectManager::finished, &app, &QCoreApplication::exit);

    int result = 1;
    if (flags.clean()) {
        QTimer::singleShot(1, &manager, &ProjectManager::clean);
        result = app.exec();
        qInfo() << "Cleaning took:" << timer.elapsed() << "ms";
    } else {
        if (!flags.qtDir().isEmpty() && (flags.qtDir() != manager.qtDir()))
            manager.setQtDir(flags.qtDir());
        QTimer::singleShot(1, &manager, &ProjectManager::start);

        if (flags.run()) {
            qInfo() << "Running compiled binary";
        }

        result = app.exec();
        qInfo() << "Build took:" << timer.elapsed() << "ms";
    }

    return result;
}
