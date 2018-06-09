#ifndef LIBRARY_GLOBAL_H
#define LIBRARY_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBRARY_LIBRARY)
#  define LIBRARYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBRARYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBRARY_GLOBAL_H
