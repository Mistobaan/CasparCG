#pragma once

#include <QtCore/QtGlobal>

#if defined(CORE_LIBRARY)
    #define CORE_EXPORT Q_DECL_EXPORT
#else
    #define CORE_EXPORT Q_DECL_IMPORT
#endif
