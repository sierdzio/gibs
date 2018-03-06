#include "projectmanager.h"
#include "fileparser.h"

#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCryptographicHash>

// TODO: add categorized logging!
#include <QDebug>

ProjectManager::ProjectManager(const Flags &flags, QObject *parent)
    : QObject(parent), mFlags(flags)
{
    mQtDir = mFlags.qtDir;
}

ProjectManager::~ProjectManager()
{
}

void ProjectManager::setQtDir(const QString &qtDir)
{
    if (qtDir == mQtDir) {
        return;
    }

    qInfo() << "Setting Qt directory to:" << qtDir;
    mQtDir = qtDir;

    // TODO: upgrade mQtLibs and mQtIncludes
}

QString ProjectManager::qtDir() const
{
    return mQtDir;
}

void ProjectManager::start()
{
    // First, check if any files need to be recompiled
    if (mCacheEnabled) {
        for (const auto &cached : mParsedFiles.values()) {
            // Check if object file exists. If somebody removed it, or used
            // --clean, then we have to recompile!

            // TODO: handle MOC and QRC this way, too. Maybe unify everything
            // under mParsedFiles?

            if (isFileDirty(cached.path, mFlags.quickMode)) {
                if (cached.type == FileInfo::Cpp) {
                    parseFile(cached.path);
                } else if (cached.type == FileInfo::QRC) {
                    onRunTool(Tags::rcc, QStringList({ cached.path }));
                }
            } else if (!cached.objectFile.isEmpty()) {
                // There should be an object file on disk - let's check
                const QFileInfo objFile(cached.objectFile);
                if (!objFile.exists()) {
                    qDebug() << "Object file missing - recompiling";
                    compile(cached.path);
                }
            } else if (!cached.generatedObjectFile.isEmpty()) {
                // There should be an object file on disk - let's check
                const QFileInfo objFile(cached.generatedObjectFile);
                if (!objFile.exists()) {
                    qDebug() << "Generated object file missing - recompiling";
                    // TODO: also check and regenerate the generated file before
                    // compiling it!

                    const QFileInfo genFile(cached.generatedFile);
                    if (!genFile.exists()) {
                        qDebug() << "Generated file missing - regenerating";
                        if (cached.type == FileInfo::Cpp) {
                            // Moc file needs to be regenerated
                            onRunMoc(cached.path);
                        } else if (cached.type == FileInfo::QRC) {
                            // QRC c++ file needs to be regenerated
                            onRunTool(Tags::rcc, QStringList({ cached.path }));
                        }
                    }

                    //compile(cached.generatedFile);
                }
            }
        }
    } else {
        onParseRequest(mFlags.inputFile);
    }
    // Parsing done, link it!
    link();    
    saveCache();

    emit finished();
}

void ProjectManager::clean()
{
    if (!mQtModules.isEmpty()) {
        qInfo() << "Cleaning MOC and QRC files";
        const QString moc("moc_predefs.h");
        if (QFile::exists(moc)) {
            QFile::remove(moc);
        }
    }

    for (const auto &info : mParsedFiles.values()) {
        if (!info.objectFile.isEmpty())
            removeFile(info.objectFile);
        if (!info.generatedFile.isEmpty())
            removeFile(info.generatedFile);
        if (!info.generatedObjectFile.isEmpty())
            removeFile(info.generatedObjectFile);
    }

    emit finished();
}

/*!
 * Save necessary build info into IBS cache file
 */
void ProjectManager::saveCache() const
{
    QFile file(Tags::ibsCacheFileName);

    if (file.exists())
        file.remove();

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qFatal("Could not open IBS cache file for writing!");
    }

    QJsonObject mainObject;

    mainObject.insert(Tags::targetName, mTargetName);
    mainObject.insert(Tags::targetType, mTargetType);
    mainObject.insert(Tags::targetLib, mTargetLibType);
    mainObject.insert(Tags::qtDir, mQtDir);
    mainObject.insert(Tags::inputFile, mFlags.inputFile);
    mainObject.insert(Tags::qtModules, QJsonArray::fromStringList(mQtModules));
    mainObject.insert(Tags::defines, QJsonArray::fromStringList(mCustomDefines));
    mainObject.insert(Tags::includes, QJsonArray::fromStringList(mCustomIncludes));
    mainObject.insert(Tags::libs, QJsonArray::fromStringList(mCustomLibs));

    QJsonArray filesArray;
    for (const auto &file : mParsedFiles.keys()) {
        const FileInfo &fi = mParsedFiles.value(file);
        if (!fi.isEmpty())
            filesArray.append(fi.toJsonArray());
    }
    mainObject.insert(Tags::parsedFiles, filesArray);

    // TODO: save also all tools that need to be run!

    const QJsonDocument document(mainObject);
    // TODO: switch back to binary format after testing
    //const auto data = document.toBinaryData();
    const auto data = document.toJson();
    const auto result = file.write(data);

    if (data.size() != result) {
        qWarning() << "Not all cache data has been saved." << data.size()
                   << "vs" << result;
    }
}

/*!
 * Checks if \a file has changed since last compilation. Returns true if it has,
 * or if it is not in cache.
 *
 * If returns true, \a file will be recompiled.
 */
bool ProjectManager::isFileDirty(const QString &file, const bool isQuickMode) const
{
    const QFileInfo realFile(file);

    if (!realFile.exists()) {
        qInfo() << "File has vanished!" << file;
    }

    const FileInfo cachedFile(mParsedFiles.value(file));

    if (realFile.created() != cachedFile.dateCreated) {
        qDebug() << "Different creation date. Recompiling."
                 << file << realFile.created() << cachedFile.dateCreated;
        return true;
    }

    if (realFile.lastModified() != cachedFile.dateModified) {
        qDebug() << "Different modification date. Recompiling."
                 << file << realFile.lastModified() << cachedFile.dateModified;
        return true;
    }

    if (!isQuickMode) {
        // Deep verification is off - for now
//        QFile fileData(file);
//        fileData.open(QFile::ReadOnly | QFile::Text);
//        const auto checksum = QCryptographicHash::hash(fileData.readAll(), QCryptographicHash::Sha1);
//        if (checksum != cachedFile.checksum) {
//            qDebug() << "Different checksum. Recompiling."
//                     << file << checksum << cachedFile.checksum;
//            return true;
//        }
    }

    return false;
}

/*!
 * Load necessary build info from IBS cache file
 */
void ProjectManager::loadCache()
{
    QFile file(Tags::ibsCacheFileName);

    if (!file.exists()) {
        qInfo("Cannot find IBS cache file");
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qFatal("Could not open IBS cache file for reading!");
    }

    qInfo() << "Loading ibs cache file" << Tags::ibsCacheFileName;

    //const auto document = QJsonDocument::fromBinaryData(file.readAll());
    const auto document = QJsonDocument::fromJson(file.readAll());
    const QJsonObject mainObject(document.object());

    onTargetName(mainObject.value(Tags::targetName).toString());
    onTargetType(mainObject.value(Tags::targetType).toString());
    mTargetLibType = mainObject.value(Tags::targetLib).toString();
    mQtDir = mainObject.value(Tags::qtDir).toString();
    //mFlags.inputFile = mainObject.value(Tags::inputFile).toString();
    onQtModules(jsonArrayToStringList(mainObject.value(Tags::qtModules).toArray()));
    onDefines(jsonArrayToStringList(mainObject.value(Tags::defines).toArray()));
    onIncludes(jsonArrayToStringList(mainObject.value(Tags::includes).toArray()));
    onLibs(jsonArrayToStringList(mainObject.value(Tags::libs).toArray()));

    const QJsonArray filesArray = mainObject.value(Tags::parsedFiles).toArray();
    for (const auto &file : filesArray) {
        FileInfo fileInfo;
        fileInfo.fromJsonArray(file.toArray());
        mParsedFiles.insert(fileInfo.path, fileInfo);
    }

    mCacheEnabled = true;
}

void ProjectManager::onParsed(const QString &file, const QString &source,
                              const QByteArray &checksum,
                              const QDateTime &modified,
                              const QDateTime &created)
{
    // Update parsed file info
    FileInfo info = mParsedFiles.value(file);
    info.type = FileInfo::Cpp;
    info.path = file;
    info.checksum = checksum;
    info.dateModified = modified;
    info.dateCreated = created;

    // Compile source file, if present
    if (!source.isEmpty() and source == file) {
        info.objectFile = compile(source);
    }

    mParsedFiles.insert(file, info);
}

void ProjectManager::onParseRequest(const QString &file, const bool force)
{
    // Skip files which we have parsed already
    if (!force and mParsedFiles.contains(file))
        return;

    // Find file in include dirs
    const QString selectedFile(findFile(file, mCustomIncludes));

    if (selectedFile.isEmpty()) {
        qWarning() << "Could not find file:" << file;
        return;
    }

    // Skip again, because name could have changed
    if (!force and mParsedFiles.contains(selectedFile))
        return;

    // Prevent file from being parsed twice
    mParsedFiles.insert(selectedFile, FileInfo());
    parseFile(selectedFile);
}

bool ProjectManager::onRunMoc(const QString &file)
{
    if (mQtDir.isEmpty()) {
        qFatal("Can't run MOC because Qt dir is not set. See 'ibs --help' for "
               "more info.");
    }

    if (mQtIsMocInitialized == false) {
        if (initializeMoc() == false)
            return false;
    }

    const QFileInfo header(file);
    const QString mocFile("moc_" + header.baseName() + ".cpp");
    const QString compiler(mQtDir + "/bin/moc");

    QStringList arguments;
    arguments.append(mQtDefines);
    arguments.append({ "--include", "moc_predefs.h" });
    arguments.append(mQtIncludes);
    // TODO: GCC includes!
    arguments.append({ file, "-o", mocFile });

    if (runProcess(compiler, arguments)) {
        FileInfo info = mParsedFiles.value(file);
        info.path = file;
        info.generatedFile = mocFile;
        info.generatedObjectFile = compile(mocFile);
        mParsedFiles.insert(file, info);
        return true;
    }

    return false;
}

void ProjectManager::onTargetName(const QString &target)
{
    qInfo() << "Setting target name:" << target;
    mTargetName = target;
}

void ProjectManager::onTargetType(const QString &type)
{
    qInfo() << "Setting target type:" << type;
    mTargetType = type;
}

void ProjectManager::onQtModules(const QStringList &modules)
{
    QStringList mod(mQtModules);
    mod += modules;
    mod.removeDuplicates();

    if (mod != mQtModules) {
        qInfo() << "Updating required Qt module list:" << mod;
        updateQtModules(mod);
    }
}

void ProjectManager::onDefines(const QStringList &defines)
{
    mCustomDefines += defines;
    mCustomDefines.removeDuplicates();
    for (const auto &define : qAsConst(mCustomDefines)) {
        const QString def("-D" + define);
        if (!mCustomDefineFlags.contains(def)) {
            mCustomDefineFlags.append(def);
        }
    }
    qInfo() << "Updating custom defines:" << mCustomDefines;
}

void ProjectManager::onIncludes(const QStringList &includes)
{
    mCustomIncludes += includes;
    mCustomIncludes.removeDuplicates();
    for(const auto &incl : qAsConst(mCustomIncludes)) {
        const QString &inc("-I" + incl);
        if (!mCustomIncludeFlags.contains(inc)) {
            mCustomIncludeFlags.append(inc);
        }
    }
    qInfo() << "Updating custom includes:" << mCustomIncludes;
}

void ProjectManager::onLibs(const QStringList &libs)
{
    mCustomLibs += libs;
    mCustomLibs.removeDuplicates();
    qInfo() << "Updating custom libs:" << mCustomLibs;
}

void ProjectManager::onRunTool(const QString &tool, const QStringList &args)
{
    if (tool == Tags::rcc) {
        // -name qml qml.qrc -o qrc_qml.cpp
        for (const auto &qrcFile : qAsConst(args)) {
            const QFileInfo file(qrcFile);
            const QString cppFile("qrc_" + file.baseName() + ".cpp");
            const QStringList arguments { "-name", file.baseName(), qrcFile,
                                        "-o", cppFile };

            runProcess(mQtDir + "/bin/" + tool, arguments);

            FileInfo info = mParsedFiles.value(qrcFile);
            info.type = FileInfo::QRC;
            info.path = qrcFile;
            info.dateModified = file.lastModified();
            info.dateCreated = file.created();
            info.generatedFile = cppFile;
            info.generatedObjectFile = compile(cppFile);
            mParsedFiles.insert(qrcFile, info);
        }
    } else if (tool == Tags::uic) {
        // TODO: add uic support
    } else {
        // TODO: add any tool support
    }
}

/*!
 * Compiles \a file and returns the name of the generated object file.
 */
QString ProjectManager::compile(const QString &file)
{
    const QFileInfo info(file);
    const QString objectFile(info.baseName() + ".o");
    const QString compiler("g++");

    if (!mQtModules.isEmpty()) {
        if (mQtDir.isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }
    }

    qInfo() << "Compiling:" << file << "into:" << objectFile;
    // TODO: add ProjectManager class and schedule compilation there (threaded!).
    QStringList arguments { "-c", "-pipe", "-g", "-D_REENTRANT", "-fPIC", "-Wall", "-W", };

    arguments.append(mCustomDefineFlags);
    arguments.append(mQtDefines);
    arguments.append(mQtIncludes);
    arguments.append(mCustomIncludeFlags);
    arguments.append({ "-o", objectFile, file });

    if (runProcess(compiler, arguments)) {
        return objectFile;
    }

    return QString();
}

bool ProjectManager::link() const
{
    QStringList objectFiles;

    for (const auto &info : mParsedFiles.values()) {
        if (!info.objectFile.isEmpty())
            objectFiles.append(info.objectFile);
        if (!info.generatedObjectFile.isEmpty())
            objectFiles.append(info.generatedObjectFile);
    }

    qInfo() << "Linking:" << objectFiles;
    const QString compiler("g++");
    QStringList arguments;

    if (mTargetType == Tags::targetLib) {
        if (mTargetLibType == Tags::targetLibDynamic) {
            arguments.append({ "-shared", "-Wl,-soname,lib" + mTargetName + ".so.1",
                               "-o", "lib" + mTargetName + ".so.1.0.0"});
        }
    } else {
        arguments.append({ "-o", mTargetName });
    }

    arguments.append(objectFiles);

    if (!mQtModules.isEmpty()) {
        if (mQtDir.isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }

        arguments.append(mQtLibs);
    }

    arguments.append(mCustomLibs);

    return runProcess(compiler, arguments);
}

void ProjectManager::parseFile(const QString &file)
{
    FileParser parser(file, mCustomIncludes);
    connect(&parser, &FileParser::parsed, this, &ProjectManager::onParsed);
    connect(&parser, &FileParser::parseRequest, this, &ProjectManager::onParseRequest);
    connect(&parser, &FileParser::runMoc, this, &ProjectManager::onRunMoc);
    connect(&parser, &FileParser::targetName, this, &ProjectManager::onTargetName);
    connect(&parser, &FileParser::targetType, this, &ProjectManager::onTargetType);
    connect(&parser, &FileParser::qtModules, this, &ProjectManager::onQtModules);
    connect(&parser, &FileParser::defines, this, &ProjectManager::onDefines);
    connect(&parser, &FileParser::includes, this, &ProjectManager::onIncludes);
    connect(&parser, &FileParser::libs, this, &ProjectManager::onLibs);
    connect(&parser, &FileParser::runTool, this, &ProjectManager::onRunTool);

    const bool result = parser.parse();

    // TODO: stop if result is false
    Q_UNUSED(result);
}

void ProjectManager::updateQtModules(const QStringList &modules)
{
    mQtModules = modules;
    mQtIncludes.clear();
    mQtLibs.clear();
    mQtDefines.clear();

    for(const QString &module : qAsConst(mQtModules)) {
        mQtDefines.append("-DQT_" + module.toUpper() + "_LIB");
    }

    mQtIncludes.append("-I" + mQtDir + "/include");
    mQtIncludes.append("-I" + mQtDir + "/mkspecs/linux-g++");

    // TODO: pre-capitalize module letters to do both loops faster
    for(const QString &module : qAsConst(mQtModules)) {
        const QString dir("-I" + mQtDir + "/include/Qt");
        if (module == Tags::quickcontrols2) {
            mQtIncludes.append(dir + "QuickControls2");
        } else if (module == Tags::quickwidgets) {
            mQtIncludes.append(dir + "QuickWidgets");
        } else {
            mQtIncludes.append(dir + capitalizeFirstLetter(module));
        }
    }

    mQtLibs.append("-Wl,-rpath," + mQtDir + "/lib");
    mQtLibs.append("-L" + mQtDir + "/lib");

    for(const QString &module : qAsConst(mQtModules)) {
        // TODO: use correct mkspecs
        // TODO: use qmake -query to get good paths
        const QString lib("-lQt5");
        if (module == Tags::quickcontrols2) {
            mQtLibs.append(lib + "QuickControls2");
        } else if (module == Tags::quickwidgets) {
            mQtLibs.append(lib + "QuickWidgets");
        } else {
            mQtLibs.append(lib + capitalizeFirstLetter(module));
        }
    }

    mQtLibs.append("-lpthread");
}

bool ProjectManager::initializeMoc()
{
    qInfo() << "Initializig MOC";
    const QString compiler("g++");
    const QStringList arguments({ "-pipe", "-g", "-Wall", "-W", "-dM", "-E",
                            "-o", "moc_predefs.h",
                            mQtDir + "/mkspecs/features/data/dummy.cpp" });
    mQtIsMocInitialized = runProcess(compiler, arguments);
    return mQtIsMocInitialized;
}

/*!
 * TODO: run asynchronously in a thread pool.
 */
bool ProjectManager::runProcess(const QString &app, const QStringList &arguments) const
{
    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    qDebug() << "Running:" << app << arguments.join(" ");
    process.start(app, arguments);
    process.waitForFinished(5000);
    const int exitCode = process.exitCode();
    if (exitCode == 0) {
        return true;
    }

    qDebug() << "Process error:" << process.errorString() << exitCode;
    return false;
}

QString ProjectManager::capitalizeFirstLetter(const QString &string) const
{
    return (string[0].toUpper() + string.mid(1));
}

QString ProjectManager::findFile(const QString &file, const QStringList &includeDirs) const
{
    QString result;
    const QFileInfo fileInfo(file);
    result = fileInfo.path() + "/" + fileInfo.fileName();

    // Search through include paths
    if (!QFileInfo(result).exists()) {
        for (const QString &inc : qAsConst(includeDirs)) {
            result = inc + "/" + fileInfo.fileName();
            if (QFileInfo(result).exists()) {
                qDebug() << "Found file in include paths!" << result;
                break;
            }
        }
    }

    return result;
}

QStringList ProjectManager::jsonArrayToStringList(const QJsonArray &array) const
{
    QStringList result;

    for (const auto &value : array) {
        result.append(value.toString());
    }

    return result;
}

void ProjectManager::removeFile(const QString &path) const
{
    if (QFile::exists(path)) {
        qInfo() << "Removing:" << path;
        QFile::remove(path);
    }
}
