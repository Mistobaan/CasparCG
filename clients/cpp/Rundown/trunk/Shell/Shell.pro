#-------------------------------------------------
#
# Project created by QtCreator 2011-04-07T13:50:44
#
#-------------------------------------------------

QT += core gui sql

TARGET = client
TEMPLATE = app

HEADERS += \

SOURCES += \
    Main.cpp \

FORMS += \

RESOURCES += \
    Resource.qrc

OTHER_FILES += \
    Images/CasparCG.ico \
    Windows.rc

RC_FILE = Windows.rc

INCLUDEPATH += $$PWD/../../dependencies/boost
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../dependencies/boost/stage/lib/win32/ -lboost_date_time-mgw44-mt-1_47 -lboost_system-mgw44-mt-1_47 -lboost_thread-mgw44-mt-1_47 -lboost_filesystem-mgw44-mt-1_47 -lboost_chrono-mgw44-mt-1_47 -lws2_32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../dependencies/boost/stage/lib/win32/ -lboost_date_time-mgw44-mt-d-1_47 -lboost_system-mgw44-mt-d-1_47 -lboost_thread-mgw44-mt-d-1_47 -lboost_filesystem-mgw44-mt-d-1_47 -lboost_chrono-mgw44-mt-d-1_47 -lws2_32

DEPENDPATH += $$PWD/../Caspar $$OUT_PWD/../Caspar
INCLUDEPATH += $$PWD/../Caspar $$OUT_PWD/../Caspar
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Caspar/release/ -lcaspar
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Caspar/debug/ -lcaspar
else:unix: LIBS += -L$$OUT_PWD/../Caspar/ -lcaspar

DEPENDPATH += $$PWD/../Gpi $$OUT_PWD/../Gpi
INCLUDEPATH += $$PWD/../Gpi $$OUT_PWD/../Gpi
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Gpi/release/ -lgpi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Gpi/debug/ -lgpi
else:unix: LIBS += -L$$OUT_PWD/../Gpi/ -lgpi

DEPENDPATH += $$PWD/../Common $$OUT_PWD/../Common
INCLUDEPATH += $$PWD/../Common $$OUT_PWD/../Common
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Common/release/ -lcommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Common/debug/ -lcommon
else:unix: LIBS += -L$$OUT_PWD/../Common/ -lcommon

DEPENDPATH += $$PWD/../Core $$OUT_PWD/../Core
INCLUDEPATH += $$PWD/../Core $$OUT_PWD/../Core
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../Core/ -lcore

DEPENDPATH += $$PWD/../Widgets $$OUT_PWD/../Widgets
INCLUDEPATH += $$PWD/../Widgets $$OUT_PWD/../Widgets
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Widgets/release/ -lwidgets
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Widgets/debug/ -lwidgets
else:unix: LIBS += -L$$OUT_PWD/../Widgets/ -lwidgets

DEPENDPATH += $$PWD/../../dependencies/gpio-client/include
INCLUDEPATH += $$PWD/../../dependencies/gpio-client/include
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../dependencies/gpio-client/lib/win32/release/ -lgpio-client
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../dependencies/gpio-client/lib/win32/debug/ -lgpio-client
