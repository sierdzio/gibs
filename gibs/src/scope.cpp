#include "scope.h"
#include "gibs.h"
#include "tags.h"
#include "metaprocess.h"
#include "fileparser.h"

#include <QDirIterator>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QProcess>

#include <QDebug>

Scope::Scope(const QString &name,
             const QString &relativePath,
             const Flags &flags,
             const QHash<QString, Gibs::Feature> &features,
             QObject *parent)
    : QObject(parent),
      mFlags(flags),
      mRelativePath(relativePath),
      mName(name),
      mId(QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha1)),
      mFeatures(features)
{
    addIncludePaths({"."});

    // Pre-set target name. If this (sub)project defines it's own name,
    // this pre-set name will be replaced.
    setTargetName(QFileInfo(name).absoluteDir().dirName());
    qDebug() << "Target name is:" << targetName();
}

// Protected constructor - used in fromJson().
Scope::Scope(const QByteArray &id,
             const QString &name,
             const QString &relativePath,
             const Flags &flags)
    : QObject(nullptr), mFlags(flags),
      mRelativePath(relativePath),
      mName(name), mId(id)
{
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
    const auto files = mParsedFiles.values();
    for (const auto &file : files) {
        if (!file.isEmpty())
            filesArray.append(file.toJsonArray());
    }    

    QJsonArray scopesArray;
    for (const auto &scope : qAsConst(mScopeDependencyIds)) {
        scopesArray.append(QString(scope.toHex()));
    }

    object.insert(Tags::scopeId, QString(id().toHex()));
    object.insert(Tags::scopeName, mName);
    object.insert(Tags::relativePath, mRelativePath);
    //object.insert(Tags::prefix, mFlags.prefix());
    //object.insert(Tags::qtDir, mFlags.qtDir());
    //object.insert(Tags::parse_whole_files, mFlags.parseWholeFiles());
    object.insert(Tags::parsedFiles, filesArray);
    object.insert(Tags::scopeTargetName, mTargetName);
    object.insert(Tags::targetType, mTargetType);
    object.insert(Tags::targetLibType, mTargetLibType);
    object.insert(Tags::scopeDependencies, scopesArray);
    object.insert(Tags::qtModules, QJsonArray::fromStringList(mQtModules));
    object.insert(Tags::defines, QJsonArray::fromStringList(mCustomDefines));
    object.insert(Tags::includes, QJsonArray::fromStringList(mCustomIncludes));
    object.insert(Tags::libs, QJsonArray::fromStringList(mCustomLibs));

    return object;
}

/*!
 * Constructs new Scope from \a json data and returns a pointer to it. The caller
 * is responsible for deleting the pointer.
 */
Scope *Scope::fromJson(const QJsonObject &json, const Flags &flags)
{
    Scope *scope = new Scope(QByteArray::fromHex(json.value(Tags::scopeId)
                                                 .toString().toLatin1()),
                             json.value(Tags::scopeName).toString(),
                             json.value(Tags::relativePath).toString(),
                             flags);
    const QJsonArray filesArray = json.value(Tags::parsedFiles).toArray();
    for (const auto &file : filesArray) {
        FileInfo fileInfo;
        fileInfo.fromJsonArray(file.toArray());
        scope->mParsedFiles.insert(fileInfo.path, fileInfo);
    }

    const QJsonArray scopesArray = json.value(Tags::scopeDependencies).toArray();
    for (const auto &scopeId : scopesArray) {
        scope->mScopeDependencyIds.append(QByteArray::fromHex(scopeId.toString().toLatin1()));
        // TODO: notify ProjectManager that it needs to connect the scopes!
    }

    scope->setTargetName(json.value(Tags::scopeTargetName).toString());
    scope->setTargetType(json.value(Tags::targetType).toString());
    scope->mTargetLibType = json.value(Tags::targetLibType).toString();
    scope->setQtModules(
                Gibs::jsonArrayToStringList(json.value(Tags::qtModules).toArray()));
    scope->addDefines(
                Gibs::jsonArrayToStringList(json.value(Tags::defines).toArray()));
    scope->addIncludePaths(
                Gibs::jsonArrayToStringList(json.value(Tags::includes).toArray()));
    scope->addLibs(
                Gibs::jsonArrayToStringList(json.value(Tags::libs).toArray()));
    // TODO: missing some properties

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
    //qDebug() << "Merging with other scope:" << other->name();
    mFlags.setQtDir(other->qtDir());
    mQtModules = other->qtModules();
    mQtIsMocInitialized = other->qtIsMocInitialized();
    mQtIncludes = other->qtIncludes();
    mQtDefines = other->qtDefines();
    mQtLibs = other->qtLibs();
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
    if (!mScopeDependencyIds.contains(other->id()))
        mScopeDependencyIds.append(other->id());
    if (!mScopeDependencies.contains(other))
        mScopeDependencies.append(other);
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
    const auto fileName = QFileInfo(path).fileName();
    const auto files = mParsedFiles.keys();
    for (const auto &file : files) {
        if (QFileInfo(file).fileName() == fileName) {
            return true;
        }
    }

    return false;
    //return mParsedFiles.contains(path);
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
    // TODO: improve compiler detection!
    const QString compiler(info.suffix() == "c"? "gcc" : "g++");

    if (!qtModules().isEmpty()) {
        if (mFlags.qtDir().isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }
    }


    //qInfo() << "Compiling:" << file << "into:" << objectFile;
    // TODO: add ProjectManager class and schedule compilation there (threaded!).
    QStringList arguments { "-c", "-pipe", "-D_REENTRANT", "-fPIC", "-Wall", "-W", };

    if (mFlags.debugBuild()) {
        arguments.append("-g");
    }

    if (mFlags.releaseBuild()) {
        arguments.append("-O2");
    }

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
    const auto parsed = parsedFiles();
    for (const auto &info : parsed) {
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
            arguments.append({ "-shared", "-Wl,-soname,lib"
                               + targetName() + ".so."
                               + QString::number(mVersion.majorVersion()),
                               "-o", mFlags.prefix() + "/" + "lib"
                               + targetName() + ".so."
                               + mVersion.toString()
                             });
        } else if (targetLibType() == Tags::targetLibStatic) {
            // Run ar to create the static library file
            MetaProcessPtr mp = MetaProcessPtr::create();
            mp->file = "lib" + targetName() + ".a";
            mp->fileDependencies = findAllDependencies();
            mp->scopeDepenencies = mScopeDependencyIds;
            mProcessQueue.append(mp);
            emit runProcess("ar", QStringList {
                                "cqs",
                                mp->file,
                                targetName() + ".o"
                            }, mp);
            return;
        }
    } else {
        arguments.append({ "-o", mFlags.prefix() + "/" + targetName() });
    }

    arguments.append(objectFiles);

    if (!qtModules().isEmpty()) {
        if (mFlags.qtDir().isEmpty()) {
            qFatal("Qt dir not set, but this is a Qt project! Specify Qt dir "
                   "with --qt-dir argument");
        }

        arguments.append(qtLibs());
    }

    arguments.append(customLibs());

    MetaProcessPtr mp = MetaProcessPtr::create();
    mp->file = targetName();
    mp->fileDependencies = findAllDependencies();
    mp->scopeDepenencies = mScopeDependencyIds;
    mProcessQueue.append(mp);
    emit runProcess(compiler, arguments, mp);

    if (targetType() == Tags::targetLib) {
        if (targetLibType() == Tags::targetLibDynamic) {
            // Create unversioned symlink to library
            MetaProcessPtr mp = MetaProcessPtr::create();
            mp->file = targetName() + ".so."
                    + QString::number(mVersion.majorVersion());
            mp->fileDependencies = findAllDependencies();
            mp->scopeDepenencies = mScopeDependencyIds;
            mProcessQueue.append(mp);
            emit runProcess("ln", QStringList {
                                "-s",
                                mFlags.prefix() + "/" + "lib"
                                + targetName() + ".so."
                                + mVersion.toString(),
                                mFlags.prefix() + "/" + "lib"
                                + targetName() + ".so"
                                //mPrefix + "/" + "lib" + targetName() + ".so."
                                //+ QString::number(mVersion.majorVersion())
                            }, mp);
        }
    }
}

void Scope::deploy()
{
    const QString deployTool = mFlags.deployTool();
    QString suffix;
    QStringList arguments;

    MetaProcessPtr mp = MetaProcessPtr::create();
    mp->file = targetName() + "." + suffix;
    mp->fileDependencies = findAllDependencies();
    mp->scopeDepenencies = mScopeDependencyIds;
    mProcessQueue.append(mp);

    if (deployTool.endsWith("AppImage")) {
        suffix = "AppImage";
        // TODO: parametrize, of course! Add .desktop file support!
        arguments.append({
                             mFlags.prefix() + "/" + targetName(),
                             "-verbose=1",
                             "-qmake=\"" + qtDir() + "/bin/qmake\"",
                             "-no-translations",
                             "-no-copy-copyright-files",
                             "-appimage"
                         });
    }

    emit runProcess(mFlags.deployTool(), arguments, mp);
}

void Scope::parseFile(const QString &file)
{
    FileParser parser(file, mFlags.parseWholeFiles(), this);
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

    // Skip files from subprojects
    // TODO: features are fine
    if (isFromSubproject(file)) {
        //qDebug() << "SKIPPING from subproject!" << file;
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

    // Skip files from subprojects
    // TODO: features are fine
    if (isFromSubproject(selectedFile)) {
        //qDebug() << "SKIPPING from subproject!" << selectedFile;
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

    if (mFlags.qtDir().isEmpty()) {
        emit error("Can't run MOC because Qt dir is not set. See 'ibs --help' "
                   "for more info.");
    }

    if (qtIsMocInitialized() == false) {
        if (initializeMoc() == false)
            return false;
    }

    const QFileInfo header(file);
    const QString mocFile("moc_" + header.baseName() + ".cpp");
    const QString compiler(mFlags.qtDir() + "/bin/moc");
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
            emit runProcess(mFlags.qtDir() + "/bin/" + tool, arguments, mp);

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

void Scope::onFeature(const QString &name, const bool isOn)
{
    Gibs::Feature result;
    if (mFeatures.contains(name)) {
        auto feat = mFeatures.value(name);
        feat.defined = true;
        mFeatures.insert(name, feat);
        result = feat;
    } else {
        Gibs::Feature feat;
        feat.enabled = isOn;
        feat.defined = true;
        feat.name = name;
        feat.define = Gibs::normalizeFeatureName(name);
        result = feat;
    }

    emit feature(result);
    //qDebug() << "Processed feature:" << result.name << result.define << result.defined << result.enabled;

    if (result.enabled) {
        addDefines(QStringList {result.define});
    }
}

QString Scope::findFile(const QString &file) const
{
    return findFile(file, mCustomIncludes);
}

QString Scope::findFile(const QString &file, const QStringList &includeDirs) const
{
    QString result(file);

    //qDebug() << "Looking for:" << file;

    // If file exists, just return it right away
    // TODO: check is this does not save absolute path to cache
    if (QFileInfo::exists(result)) {
        //qDebug() << "RETURNING 1:" << result;
        return result;
    }

    if (file.contains(mRelativePath))
        result = file;
    else
        result = mRelativePath + "/" + file;

    //qDebug() << "Pre-sanitize:" << result;

    // Sanitize the path
    //result = QDir(mRelativePath).relativeFilePath(
    //            QFileInfo(result).canonicalFilePath());

    // Search through include paths
    if (QFileInfo::exists(result)) {
        //qDebug() << "RETURNING 2:" << result;
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

bool Scope::isFromSubproject(const QString &file) const
{
    for (const auto &scope : qAsConst(mScopeDependencies)) {
        if (scope->isParsed(file))
            return true;
    }

    return false;
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

    if(mFlags.debugBuild()) {
        mQtDefines.append("-DQT_QML_DEBUG");
    }

    if (mFlags.releaseBuild()) {
        mQtDefines.append("-DQT_NO_DEBUG");
        mQtLibs.append("-Wl,-O1 ");
    }

    mQtIncludes.append("-I" + mFlags.qtDir() + "/include");
    mQtIncludes.append("-I" + mFlags.qtDir() + "/mkspecs/linux-g++");

    // TODO: pre-capitalize module letters to do both loops faster
    for(const QString &module : qAsConst(mQtModules)) {
        const QString dir("-I" + mFlags.qtDir() + "/include/Qt");
        if (module == Tags::quickcontrols2) {
            mQtIncludes.append(dir + "QuickControls2");
        } else if (module == Tags::quickwidgets) {
            mQtIncludes.append(dir + "QuickWidgets");
        } else {
            mQtIncludes.append(dir + Gibs::capitalizeFirstLetter(module));
        }
    }

    mQtLibs.append("-Wl,-rpath," + mFlags.qtDir() + "/lib");
    mQtLibs.append("-L" + mFlags.qtDir() + "/lib");

    for(const QString &module : qAsConst(mQtModules)) {
        // TODO: use correct mkspecs
        // TODO: use qmake -query to get good paths
        const QString lib("-lQt5");
        if (module == Tags::quickcontrols2) {
            mQtLibs.append(lib + "QuickControls2");
        } else if (module == Tags::quickwidgets) {
            mQtLibs.append(lib + "QuickWidgets");
        } else {
            mQtLibs.append(lib + Gibs::capitalizeFirstLetter(module));
        }
    }

    mQtLibs.append("-lpthread");
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
        mFlags.qtDir() + "/mkspecs/features/data/dummy.cpp" });

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

QHash<QString, Gibs::Feature> Scope::features() const
{
    return mFeatures;
}

void Scope::setVersion(const QVersionNumber &version)
{
    mVersion = version;
}

QVersionNumber Scope::version() const
{
    return mVersion;
}

QVector<QByteArray> Scope::scopeDependencyIds() const
{
    return mScopeDependencyIds;
}

void Scope::setTargetLibType(const QString &targetLibType)
{
    mTargetLibType = targetLibType;
}

QString Scope::qtDir() const
{
    return mFlags.qtDir();
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
    // First, check if any files need to be recompiled
    if (fromCache) {
        const auto files = parsedFiles();
        for (const auto &cached : files) {
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

    // Linking is scheduled, deploy it!
    if (!mFlags.deployTool().isEmpty() and targetType() == Tags::targetApp) {
        // Use the deployment tool!
        deploy();
    }
}

void Scope::clean()
{
    const auto files = parsedFiles();
    for (const auto &info : files) {
        if (!info.objectFile.isEmpty())
            Gibs::removeFile(info.objectFile);
        if (!info.generatedFile.isEmpty())
            Gibs::removeFile(info.generatedFile);
        if (!info.generatedObjectFile.isEmpty())
            Gibs::removeFile(info.generatedObjectFile);
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
