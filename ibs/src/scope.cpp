#include "scope.h"
#include "ibs.h"
#include "tags.h"
#include "metaprocess.h"
#include "fileparser.h"

#include <QDirIterator>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QProcess>

#include <QDebug>

Scope::Scope(const QString &name, const QString &relativePath, const QString &prefix,
             const QString &qtDir,
             QObject *parent)
    : QObject(parent),
      mRelativePath(relativePath),
      mPrefix(prefix),
      mName(name),
      mId(QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha1)),
      mQtDir(qtDir)
{
    addIncludePaths({"."});
}

QString Scope::name() const
{
    return mName;
}

QByteArray Scope::id() const
{
    return mId;
}

QString Scope::relativePath() const
{
    return mRelativePath;
}

QJsonObject Scope::toJson() const
{
    QJsonObject object;
    QJsonArray filesArray;
    for (const auto &file : mParsedFiles.values()) {
        if (!file.isEmpty())
            filesArray.append(file.toJsonArray());
    }

    object.insert(Tags::parsedFiles, filesArray);
    object.insert(Tags::targetName, mTargetName);
    object.insert(Tags::targetType, mTargetType);
    object.insert(Tags::targetLib, mTargetLibType);
    object.insert(Tags::qtModules, QJsonArray::fromStringList(mQtModules));
    object.insert(Tags::defines, QJsonArray::fromStringList(mCustomDefines));
    object.insert(Tags::libs, QJsonArray::fromStringList(mCustomLibs));

    return object;
}

Scope *Scope::fromJson(const QJsonObject &json)
{
    Scope *scope = new Scope();
    const QJsonArray filesArray = json.value(Tags::parsedFiles).toArray();
    for (const auto &file : filesArray) {
        FileInfo fileInfo;
        fileInfo.fromJsonArray(file.toArray());
        scope->mParsedFiles.insert(fileInfo.path, fileInfo);
    }

    scope->setTargetName(json.value(Tags::targetName).toString());
    scope->setTargetType(json.value(Tags::targetType).toString());
    scope->mTargetLibType = json.value(Tags::targetLib).toString();
    scope->setQtModules(scope->jsonArrayToStringList(json.value(Tags::qtModules).toArray()));
    scope->addDefines(scope->jsonArrayToStringList(json.value(Tags::defines).toArray()));
    //onIncludes(jsonArrayToStringList(mainObject.value(Tags::includes).toArray()));
    scope->addLibs(scope->jsonArrayToStringList(json.value(Tags::libs).toArray()));

    return scope;
}

/*!
 * "Inherits" internals from \a other scope. Useful if you have a parent scope
 * (like global scope from CommandParser) and want to share the settings with
 * current scope.
 *
 * ID of \a other scope is not merged - current scope will have different ID
 * and QObject parent.
 */
void Scope::mergeWith(const ScopePtr &other)
{
    mQtDir = other->qtDir();
    mQtModules = other->qtModules();
    mQtIsMocInitialized = other->qtIsMocInitialized();
    mQtIncludes = other->qtIncludes();
    mQtDefines = other->qtDefines();
    mCustomDefines = other->mCustomDefines; //
    mCustomDefineFlags = other->customDefineFlags();
    mCustomLibs = other->customLibs();
    //mTargetName;
    //mTargetType;
    //mTargetLibType;
    mCustomIncludes = other->mCustomIncludes; //
    mCustomIncludeFlags = other->customIncludeFlags();
    //mParsedFiles;
}

void Scope::dependOn(const ScopePtr &other)
{
    mScopeDependencies.append(other->id());
}

bool Scope::isFinished() const
{
    for (const auto &mp : qAsConst(mProcessQueue)) {
        if (mp->hasFinished == false)
            return false;
    }

    return false;
}

QList<FileInfo> Scope::parsedFiles() const
{
    return mParsedFiles.values();
}

void Scope::insertParsedFile(const FileInfo &fileInfo)
{
    mParsedFiles.insert(fileInfo.path, fileInfo);
}

FileInfo Scope::parsedFile(const QString &path) const
{
    return mParsedFiles.value(path);
}

bool Scope::isParsed(const QString &path) const
{
    return mParsedFiles.contains(path);
}

void Scope::addIncludePaths(const QStringList &includes)
{
    mCustomIncludes += includes;
    mCustomIncludes.removeDuplicates();
    for(const auto &incl : qAsConst(mCustomIncludes)) {
        const QString &inc("-I"+ mRelativePath + "/" + incl);
        if (!mCustomIncludeFlags.contains(inc)) {
            mCustomIncludeFlags.append(inc);
        }
    }
    qInfo() << "Updating custom includes:" << mCustomIncludes;
}

QStringList Scope::includePaths() const
{
    return mCustomIncludes;
}

QStringList Scope::customIncludeFlags() const
{
    return mCustomIncludeFlags;
}

/*!
 * Scans \a path for all subdirectories and adds them to include dir list.
 *
 * This can be used to automatically guess include paths in a project.
 *
 * \a path can be a file - the function will search for all subdirs of the
 * directory where that file is located.
 */
void Scope::autoScanForIncludes()
{
    QStringList result;

    // Make sure we don't deal with a file
    QDirIterator it(QFileInfo(mName).path(),
                    QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        result.append(it.next());
    }

    addIncludePaths(result);
}

void Scope::setTargetName(const QString &target)
{
    mTargetName = target;
}

void Scope::setTargetType(const QString &type)
{
    mTargetType = type;
}

void Scope::setQtModules(const QStringList &modules)
{
    QStringList mod(mQtModules);
    mod += modules;
    mod.removeDuplicates();

    if (mod != mQtModules) {
        updateQtModules(mod);
    }
}

void Scope::addDefines(const QStringList &defines)
{
    mCustomDefines += defines;
    mCustomDefines.removeDuplicates();
    for (const auto &define : qAsConst(mCustomDefines)) {
        const QString def("-D" + define);
        if (!mCustomDefineFlags.contains(def)) {
            mCustomDefineFlags.append(def);
        }
    }
}

void Scope::addLibs(const QStringList &libs)
{
    mCustomLibs += libs;
    mCustomLibs.removeDuplicates();
}

QString Scope::compile(const QString &file)
{
    if (mIsError)
        return QString();

    const QFileInfo info(file);
    const QString objectFile(info.baseName() + ".o");
    const QString compiler("g++");

    if (!qtModules().isEmpty()) {
        if (mQtDir.isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }
    }


    //qInfo() << "Compiling:" << file << "into:" << objectFile;
    // TODO: add ProjectManager class and schedule compilation there (threaded!).
    QStringList arguments { "-c", "-pipe", "-g", "-D_REENTRANT", "-fPIC", "-Wall", "-W", };

    arguments.append(customDefineFlags());
    arguments.append(qtDefines());
    arguments.append(qtIncludes());
    arguments.append(customIncludeFlags());
    arguments.append({ "-o", objectFile, file });

    MetaProcessPtr mp = MetaProcessPtr::create();
    mp->file = objectFile;
    mp->fileDependencies = findDependencies(file);
    mProcessQueue.append(mp);

    emit runProcess(compiler, arguments, mp);
    return objectFile;
}

void Scope::link()
{
    if (mIsError)
        return;

    QStringList objectFiles;
    for (const auto &info : parsedFiles()) {
        if (!info.objectFile.isEmpty())
            objectFiles.append(info.objectFile);
        if (!info.generatedObjectFile.isEmpty())
            objectFiles.append(info.generatedObjectFile);
    }

    // TODO: add dependent scopes (from other subprojects)

    qInfo() << "Linking:" << objectFiles;
    const QString compiler("g++");
    QStringList arguments;

    if (targetType() == Tags::targetLib) {
        if (targetLibType() == Tags::targetLibDynamic) {
            arguments.append({ "-shared", "-Wl,-soname,lib" + targetName() + ".so.1",
                               "-o", mPrefix + "/" + "lib" + targetName() + ".so"
                               //".1.0.0"
                             });
        }
    } else {
        arguments.append({ "-o", mPrefix + "/" + targetName() });
    }

    arguments.append(objectFiles);

    if (!qtModules().isEmpty()) {
        if (mQtDir.isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }

        arguments.append(qtLibs());
    }

    arguments.append(customLibs());

    MetaProcessPtr mp = MetaProcessPtr::create();
    mp->file = targetName();
    mp->fileDependencies = findAllDependencies();
    mp->scopeDepenencies = mScopeDependencies;
    mProcessQueue.append(mp);
    emit runProcess(compiler, arguments, mp);
}

void Scope::parseFile(const QString &file)
{
    FileParser parser(file, this);
    connect(&parser, &FileParser::error, this, &Scope::error);
    connect(&parser, &FileParser::parsed, this, &Scope::onParsed);
    connect(&parser, &FileParser::parseRequest, this, &Scope::onParseRequest);
    connect(&parser, &FileParser::runMoc, this, &Scope::onRunMoc);
    connect(&parser, &FileParser::runTool, this, &Scope::onRunTool);
    connect(&parser, &FileParser::subproject, this, &Scope::subproject);
    parser.parse();
}

/*!
 * Checks if \a file has changed since last compilation. Returns true if it has,
 * or if it is not in cache.
 *
 * If returns true, \a file will be recompiled.
 */
bool Scope::isFileDirty(const QString &file, const bool isQuickMode) const
{
    const QFileInfo realFile(file);

    if (!realFile.exists()) {
        qInfo() << "File has vanished!" << file;
    }

    const FileInfo cachedFile(parsedFile(file));

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

void Scope::onParsed(const QString &file, const QString &source, const QByteArray &checksum, const QDateTime &modified, const QDateTime &created)
{
    // Update parsed file info
    FileInfo info = parsedFile(file);
    info.type = FileInfo::Cpp;
    info.path = file;
    info.checksum = checksum;
    info.dateModified = modified;
    info.dateCreated = created;

    // Compile source file, if present
    if (!source.isEmpty() and source == file) {
        info.objectFile = compile(source);
    }

    // TODO: switch to pointers and modify in-place?
    insertParsedFile(info);
    //mScopes.insert(scopeId, scope);
}

void Scope::onParseRequest(const QString &file, const bool force)
{
    if (mIsError)
        return;

    // Skip files which we have parsed already
    if (!force and isParsed(file)) {
        return;
    }

    // Find file in include dirs
    const QString selectedFile(findFile(file));

    if (selectedFile.isEmpty()) {
        qWarning() << "Could not find file:" << file;
        return;
    }

    // Skip again, because name could have changed
    if (!force and isParsed(selectedFile)) {
        return;
    }

    // Prevent file from being parsed twice
    FileInfo info;
    info.path = selectedFile;
    insertParsedFile(info);
    parseFile(selectedFile);
}

bool Scope::onRunMoc(const QString &file)
{
    if (mIsError)
        return false;

    if (mQtDir.isEmpty()) {
        emit error("Can't run MOC because Qt dir is not set. See 'ibs --help' for "
                   "more info.");
    }

    if (qtIsMocInitialized() == false) {
        if (initializeMoc() == false)
            return false;
    }

    const QFileInfo header(file);
    const QString mocFile("moc_" + header.baseName() + ".cpp");
    const QString compiler(mQtDir + "/bin/moc");
    const QString predefs("moc_predefs.h");

    QStringList arguments;
    arguments.append(qtDefines());
    arguments.append({ "--include", predefs });
    arguments.append(qtIncludes());
    // TODO: GCC includes!
    arguments.append({ file, "-o", mocFile });

    MetaProcessPtr mp = MetaProcessPtr::create();
    mp->file = mocFile;
    mp->fileDependencies.append(findDependency(predefs));
    mProcessQueue.append(mp);
    // Generate MOC file
    emit runProcess(compiler, arguments, mp);


    FileInfo info = parsedFile(file);
    info.path = mRelativePath + "/" + file;
    info.generatedFile = mocFile;
    // Compile MOC file
    info.generatedObjectFile = compile(mocFile);

    // TODO: IMPORTANT! Old code used 'file' as key for parsed file hash here,
    // new code uses 'info.path' instead, and they are different!
    insertParsedFile(info);
    //mScopes.insert(scopeId, scope);
    return true;
}

void Scope::onRunTool(const QString &tool, const QStringList &args)
{
    if (mIsError)
        return;

    if (tool == Tags::rcc) {
        // -name qml qml.qrc -o qrc_qml.cpp
        for (const auto &qrcFile : qAsConst(args)) {
            const QFileInfo file(mRelativePath + "/" + qrcFile);
            const QString cppFile("qrc_" + file.baseName() + ".cpp");
            const QStringList arguments { "-name", file.baseName(),
                        mRelativePath + "/" + qrcFile,
                        "-o", cppFile };

            qDebug() << "Running tool: rcc" << mRelativePath + "/" + qrcFile << cppFile;

            MetaProcessPtr mp = MetaProcessPtr::create();
            mp->file = cppFile;
            mProcessQueue.append(mp);
            emit runProcess(mQtDir + "/bin/" + tool, arguments, mp);

            FileInfo info = parsedFile(qrcFile);
            info.type = FileInfo::QRC;
            info.path = mRelativePath + "/" + qrcFile;
            info.dateModified = file.lastModified();
            info.dateCreated = file.created();
            info.generatedFile = cppFile;
            info.generatedObjectFile = compile(cppFile);
            insertParsedFile(info);
        }
    } else if (tool == Tags::uic) {
        // TODO: add uic support
    } else {
        // TODO: add any tool support
    }
}

//void Scope::onSubproject(const QByteArray &scopeId, const QString &path)
//{
//}

QString Scope::findFile(const QString &file) const
{
    return findFile(file, mCustomIncludes);
}

// Protected constructor - used in fromJson().
Scope::Scope()
{
}

QString Scope::findFile(const QString &file, const QStringList &includeDirs) const
{
    QString result;
    if (file.contains(mRelativePath))
        result = file;
    else
        result = mRelativePath + "/" + file;

    // Search through include paths
    if (QFileInfo::exists(result)) {
        //qDebug() << "RETURNING:" << result;
        return result;
    }

    for (const QString &inc : qAsConst(includeDirs)) {
        const QString tempResult(mRelativePath + "/" + inc + "/" + file);
        if (QFileInfo::exists(tempResult)) {
            result = tempResult;
            break;
        }
    }

    //qDebug() << "FOUND:" << result;
    return result;
}

void Scope::updateQtModules(const QStringList &modules)
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

QString Scope::capitalizeFirstLetter(const QString &string) const
{
    return (string[0].toUpper() + string.mid(1));
}

QStringList Scope::jsonArrayToStringList(const QJsonArray &array) const
{
    QStringList result;

    for (const auto &value : array) {
        result.append(value.toString());
    }

    return result;
}

MetaProcessPtr Scope::findDependency(const QString &file) const
{
    for (const MetaProcessPtr &mp : qAsConst(mProcessQueue)) {
        if (mp->file == file)
            return mp;
    }

    return MetaProcessPtr();
}

QVector<MetaProcessPtr> Scope::findDependencies(const QString &file) const
{
    QString dependencies;
    QVector<MetaProcessPtr> result;
    for (const MetaProcessPtr &mp : qAsConst(mProcessQueue)) {
        if (mp->file == file) {
            dependencies += QFileInfo(mp->file).fileName() + ", ";
            result.append(mp);
        }
    }

    qDebug() << "File" << file << "depends on:" << dependencies;

    return result;
}

QVector<MetaProcessPtr> Scope::findAllDependencies() const
{
    QString dependencies;
    QVector<MetaProcessPtr> result;
    for (const MetaProcessPtr &mp : qAsConst(mProcessQueue)) {
        dependencies += QFileInfo(mp->file).fileName() + ", ";
        result.append(mp);
    }

    qDebug() << "All dependencies:" << dependencies;
    return result;
}

bool Scope::initializeMoc()
{
    qInfo() << "Initializig MOC";
    const QString compiler("g++");
    const QString predefs("moc_predefs.h");
    const QStringList arguments({ "-pipe", "-g", "-Wall", "-W", "-dM", "-E",
                            "-o", predefs,
                            mQtDir + "/mkspecs/features/data/dummy.cpp" });

    FileInfo info;
    info.path = predefs;
    info.generatedFile = predefs;
    insertParsedFile(info);
    //mScopes.insert(scopeId, scope);
    // TODO: is predefs info lost? Check dependency resolving
    //mParsedFiles.insert(predefs, info);

    MetaProcessPtr mp = MetaProcessPtr::create();
    mp->file = predefs;
    mProcessQueue.append(mp);
    emit runProcess(compiler, arguments, mp);
    setQtIsMocInitialized(true);
    return qtIsMocInitialized();
}

void Scope::setTargetLibType(const QString &targetLibType)
{
    mTargetLibType = targetLibType;
}

QString Scope::qtDir() const
{
    return mQtDir;
}

bool Scope::qtIsMocInitialized() const
{
    return mQtIsMocInitialized;
}

void Scope::setQtIsMocInitialized(bool qtIsMocInitialized)
{
    mQtIsMocInitialized = qtIsMocInitialized;
}

void Scope::start(bool fromCache, bool isQuickMode)
{
    QByteArray tempScopeId;

    // First, check if any files need to be recompiled
    if (fromCache) {
            for (const auto &cached : parsedFiles()) {
                // Check if object file exists. If somebody removed it, or used
                // --clean, then we have to recompile!

                if (mIsError)
                    return;

                if (isFileDirty(cached.path, isQuickMode)) {
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
        //qDebug() << "I SHOULD BE HERE!" << mName;
        onParseRequest(mName);
    }

    // Parsing done, link it!
    link();
}

void Scope::clean()
{
    for (const auto &info : parsedFiles()) {
        if (!info.objectFile.isEmpty())
            Ibs::removeFile(info.objectFile);
        if (!info.generatedFile.isEmpty())
            Ibs::removeFile(info.generatedFile);
        if (!info.generatedObjectFile.isEmpty())
            Ibs::removeFile(info.generatedObjectFile);
    }

    if (!qtModules().isEmpty()) {
        qInfo() << "Cleaning MOC and QRC files";
        const QString moc("moc_predefs.h");
        if (QFile::exists(moc)) {
            QFile::remove(moc);
        }
    }
}

QStringList Scope::customLibs() const
{
    return mCustomLibs;
}

QString Scope::targetLibType() const
{
    return mTargetLibType;
}

QString Scope::targetType() const
{
    return mTargetType;
}

QString Scope::targetName() const
{
    return mTargetName;
}

QStringList Scope::qtLibs() const
{
    return mQtLibs;
}

QStringList Scope::qtIncludes() const
{
    return mQtIncludes;
}

QStringList Scope::qtDefines() const
{
    return mQtDefines;
}

QStringList Scope::customDefineFlags() const
{
    return mCustomDefineFlags;
}

QStringList Scope::qtModules() const
{
    return mQtModules;
}
