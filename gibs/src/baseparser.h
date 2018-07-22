#pragma once

#include "scope.h"

#include <QObject>
#include <QPointer>
#include <QVersionNumber>

/*!
 * \brief The BaseParser class contains basic gibs commands parsing capabilities.
 */
class BaseParser : public QObject
{
    Q_OBJECT

public:
    explicit BaseParser(Scope *scope, QObject *parent = nullptr);

signals:
    /*!
     * Emitted when pasring encounters an \a error.
     *
     * \warning Error reporting in gibs is currently not really implemented
     */
    void error(const QString &error) const;

    /*!
     * Emitted when parser detects target name command. The name is sent in
     * \a target.
     */
    void targetName(const QString &target) const;

    /*!
     * Emitted when parser detects target type command (application, library,
     * etc.). Type is sent in \a type.
     */
    void targetType(const QString &type) const;

    /*!
     * Emitted when parser detects target library type command (static, dynamic).
     * Type is sent in \a libType.
     */
    void targetLibType(const QString &libType) const;

    /*!
     * When Qt modules command is detected, it will send the list of \a modules.
     */
    void qtModules(const QStringList &modules) const;    

    /*!
     * Emitted when parser detects new compiler \a defines that need to be added
     * to the build process.
     */
    void defines(const QStringList &defines) const;

    /*!
     * Emitted when parser detects new compiler \a includes that need to be added
     * to the build process. That is, new include directories.
     */
    void includes(const QStringList &includes) const;

    /*!
     * Emitted when parser detects new \a libs that need to be added
     * to the build process (linking stage).
     */
    void libs(const QStringList &libs) const;

    /*!
     * Emitted when parser detects command asking gibs to run a \a tool with
     * given \a args.
     */
    void runTool(const QString &tool, const QStringList &args) const;    

    /*!
     * Emitted when subproject definition is detected. Sends current \a scopeId
     * and the \a path to new subproject.
     */
    void subproject(const QByteArray &scopeId, const QString &path) const;

    /*!
     * Emitted when parser detects project \a version string.
     */
    void version(const QVersionNumber &version) const;

    /*!
     * Emitted when parser detects new feature \a name and whether it is on by
     * default (\a defaultOn).
     */
    void feature(const QString &name, const bool defaultOn) const;

public slots:
    virtual bool parse() = 0;

protected:
    bool parseCommand(const QString &commandString);
    QString extractArguments(const QString &line, const QLatin1String &tag) const;

    QPointer<Scope> mScope; // TODO: maybe BaseParser can inherit from Scope?
};
