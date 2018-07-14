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

/*******************************************************************************
Copyright (C) 2018 Tomasz Siekierda

Parts of this file, as well as whole of gibs project, was written by Tomasz
Siekierda. These parts are available under license mentioned LICENSE file.
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
#include <QElapsedTimer>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

#include "mlog.h"

#include "globals.h"
#include "tags.h"
#include "flags.h"
#include "gibs.h"
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
        {Tags::pipe_flag,
        QCoreApplication::translate(scope, "Pipe C/C++ code read by gibs into compiler. This prevents files from being read twice.")},
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
        {Tags::deployer_tool,
        QCoreApplication::translate(scope, "name of deployment tool to use, for example linuxdeployqt. If specified, Gibs will search for deployer definitions in $HOME/gibs/deployers. Built-in compiler definitions are: linuxdeployqt, androiddeployqt. Deployment tool needs to be either in $PATH, or inside qtdir/bin, or specified manyally using --deployer-path"),
        QCoreApplication::translate(scope, "deployment tool name")},
        {Tags::deployer_path,
        QCoreApplication::translate(scope, "path to deployment tool, including the executable file name"),
        QCoreApplication::translate(scope, "deoliyer binary path")},
        {Tags::compiler_tool,
        QCoreApplication::translate(scope, "compiler name. If specified, Gibs will search for compiler definitions in $HOME/gibs/compilers. Built-in compiler definitions are: gcc, clang. Once Gibs learns to parse mkspecs, it will be a source of compiler configs as well"),
        QCoreApplication::translate(scope, "compiler name")},
        {Tags::cross_compile_flag,
        QCoreApplication::translate(scope, "Cross compile. You need to specify correct compiler, sysroot, toolchain etc.")},
        {Tags::sysroot,
        QCoreApplication::translate(scope, "Cross compilation sysroot"),
        QCoreApplication::translate(scope, "path")},
        {Tags::toolchain,
        QCoreApplication::translate(scope, "Cross compilation toolchain"),
        QCoreApplication::translate(scope, "path")},
        {Tags::androidNdkPath,
        QCoreApplication::translate(scope, "Android NDK path"),
        QCoreApplication::translate(scope, "path")},
        {Tags::androidNdkApi,
        QCoreApplication::translate(scope, "Android NDK API level"),
        QCoreApplication::translate(scope, "level")},
        {Tags::androidNdkAbi,
        QCoreApplication::translate(scope, "Android NDK ABI level"),
        QCoreApplication::translate(scope, "level")},
        {Tags::androidSdkPath,
        QCoreApplication::translate(scope, "Android SDK path"),
        QCoreApplication::translate(scope, "path")},
        {Tags::androidSdkApi,
        QCoreApplication::translate(scope, "Android SDK API level"),
        QCoreApplication::translate(scope, "level")},
        {Tags::jdkPath,
        QCoreApplication::translate(scope, "Java JDK path"),
        QCoreApplication::translate(scope, "path")},
    });

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    bool jobsOk = false;

    // TODO: add command line option to disable path config (pass false to flags
    // constructor
    Flags flags(true);
    flags.debugBuild = parser.isSet(Tags::debug);
    // TODO: support debug-and-release maybe.
    if (flags.debugBuild)
        flags.releaseBuild = false;
    flags.run = parser.isSet(Tags::run);
    flags.clean = parser.isSet(Tags::clean);
    flags.quickMode = parser.isSet(Tags::quick_flag);
    flags.qtAutoModules = parser.isSet(Tags::auto_qt_modules_flag);
    flags.autoIncludes = parser.isSet(Tags::auto_include_flag);
    flags.parseWholeFiles = parser.isSet(Tags::parse_whole_files);
    flags.crossCompile = parser.isSet(Tags::cross_compile_flag);
    flags.pipe = parser.isSet(Tags::pipe_flag);

    flags.setJobs(parser.value(Tags::jobs).toFloat(&jobsOk));
    flags.qtDir = Gibs::ifEmpty(parser.value(Tags::qt_dir_flag), flags.qtDir);
    flags.commands = Gibs::ifEmpty(parser.value(Tags::commands), flags.commands);
    flags.deployerName = Gibs::ifEmpty(parser.value(Tags::deployer_tool),
                                       flags.deployerName);
    flags.compilerName = Gibs::ifEmpty(parser.value(Tags::compiler_tool),
                                       flags.compilerName);
    flags.sysroot = Gibs::ifEmpty(parser.value(Tags::sysroot), flags.sysroot);
    flags.toolchain = Gibs::ifEmpty(parser.value(Tags::toolchain),
                                    flags.toolchain);
    flags.androidNdkPath = Gibs::ifEmpty(parser.value(Tags::androidNdkPath),
                                         flags.androidNdkPath);
    flags.androidNdkApi = Gibs::ifEmpty(parser.value(Tags::androidNdkApi),
                                        flags.androidNdkApi);
    flags.androidNdkAbi = Gibs::ifEmpty(parser.value(Tags::androidNdkAbi),
                                        flags.androidNdkAbi);
    flags.androidSdkPath = Gibs::ifEmpty(parser.value(Tags::androidSdkPath),
                                         flags.androidSdkPath);
    flags.androidSdkApi = Gibs::ifEmpty(parser.value(Tags::androidSdkApi),
                                        flags.androidSdkApi);
    flags.jdkPath = Gibs::ifEmpty(parser.value(Tags::jdkPath), flags.jdkPath);

    if (args.isEmpty()) {
        const QString defaultInput("main.cpp");
        if (QFileInfo::exists(defaultInput)) {
            flags.inputFile = defaultInput;
        } else {
            qFatal("No input file specified, can't continue!");
        }
    } else {
        flags.inputFile = args.at(0);
    }
    flags.setRelativePath(flags.inputFile);

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
    QObject::connect(&manager, &ProjectManager::finished,
                     &app, [=]() { flags.save(); });
    QObject::connect(&manager, &ProjectManager::finished,
                     &app, &QCoreApplication::exit);

    int result = 1;
    if (flags.clean) {
        QTimer::singleShot(1, &manager, &ProjectManager::clean);
        result = app.exec();
        qInfo() << "Cleaning took:" << timer.elapsed() << "ms";
    } else {
        if (!flags.qtDir.isEmpty() && (flags.qtDir != manager.qtDir()))
            manager.setQtDir(flags.qtDir);
        QTimer::singleShot(1, &manager, &ProjectManager::start);

        if (flags.run) {
            qInfo() << "Running compiled binary";
        }

        result = app.exec();
        qInfo() << "Build took:" << timer.elapsed() << "ms";
    }

    return result;
}
