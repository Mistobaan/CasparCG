#pragma once

#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

#if defined(CASPAR_LIBRARY)
    #define CASPAR_EXPORT Q_DECL_EXPORT
#else
    #define CASPAR_EXPORT Q_DECL_IMPORT
#endif
