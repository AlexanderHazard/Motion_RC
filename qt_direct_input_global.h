#ifndef QT_DIRECT_INPUT_GLOBAL_H
#define QT_DIRECT_INPUT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QT_DIRECT_INPUT_LIBRARY)
#  define QT_DIRECT_INPUTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QT_DIRECT_INPUTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QT_DIRECT_INPUT_GLOBAL_H
