#include "gibs.h"

void Gibs::removeFile(const QString &path) {
    if (QFile::exists(path)) {
        qInfo() << "Removing:" << path;
        QFile::remove(path);
    }
}

/*!
 * For given feature \a name (example: "tts-support") returns an upper-cased
 * and sanitized name, suitable for a c++ define (example: "TTS_SUPPORT").
 */
QString Gibs::normalizeFeatureName(const QString &name)
{
    QString result(name.toUpper());
    result = result.replace('-', '_');
    return result;
}
