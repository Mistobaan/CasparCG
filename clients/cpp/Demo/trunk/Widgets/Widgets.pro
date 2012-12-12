#-------------------------------------------------
#
# Project created by QtCreator 2011-04-07T13:50:44
#
#-------------------------------------------------

QT += core gui

TARGET = Widgets
TEMPLATE = lib

DEFINES += WIDGETS_LIBRARY

HEADERS += \
    MainWindow.h \
    AboutDialog.h \
    SqueezeWidget.h \
    RecorderWidget.h \
    Shared.h \
    BigFourWidget.h \
    StartWidget.h \
    FrameWidget.h

SOURCES += \
    MainWindow.cpp \
    AboutDialog.cpp \
    SqueezeWidget.cpp \
    RecorderWidget.cpp \
    BigFourWidget.cpp \
    StartWidget.cpp \
    FrameWidget.cpp

FORMS += \
    MainWindow.ui \
    AboutDialog.ui \
    SqueezeWidget.ui \
    RecorderWidget.ui \
    BigFourWidget.ui \
    StartWidget.ui \
    FrameWidget.ui

RESOURCES += \
    Resource.qrc

OTHER_FILES += \
    Images/ArrowUpDisabled.png \
    Images/ArrowUp.png \
    Images/ArrowDownDisabled.png \
    Images/ArrowDown.png \
    Stylesheets/Default.css \
    Stylesheets/Extended.css \
    Stylesheets/Unix.css \
    Stylesheets/Windows.css \
    Images/RadiobuttonUncheckedPressed.png \
    Images/RadiobuttonUncheckedHover.png \
    Images/RadiobuttonUnchecked.png \
    Images/RadiobuttonCheckedPressed.png \
    Images/RadiobuttonCheckedHover.png \
    Images/RadiobuttonChecked.png \
    Images/CheckboxUncheckedPressed.png \
    Images/CheckboxUncheckedHover.png \
    Images/CheckboxUnchecked.png \
    Images/CheckboxCheckedPressed.png \
    Images/CheckboxCheckedHover.png \
    Images/CheckboxChecked.png \
    Images/CasparCG.png \
    Images/ArrowLeftDisabled.png \
    Images/ArrowLeft.png \
    Images/ArrowRightDisabled.png \
    Images/ArrowRight.png \
    Images/Picture.png \
    Images/Recording.png \
    Images/Record.png \
    Images/Connection.png

DEPENDPATH += $$PWD/../Caspar $$OUT_PWD/../Caspar
INCLUDEPATH += $$PWD/../Caspar $$OUT_PWD/../Caspar
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Caspar/release/ -lCaspar
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Caspar/debug/ -lCaspar
else:unix: LIBS += -L$$OUT_PWD/../Caspar/ -lCaspar

DEPENDPATH += $$PWD/../Common $$OUT_PWD/../Common
INCLUDEPATH += $$PWD/../Common $$OUT_PWD/../Common
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Common/release/ -lCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Common/debug/ -lCommon
else:unix: LIBS += -L$$OUT_PWD/../Common/ -lCommon

DEPENDPATH += $$PWD/../Core $$OUT_PWD/../Core
INCLUDEPATH += $$PWD/../Core $$OUT_PWD/../Core
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Core/release/ -lCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Core/debug/ -lCore
else:unix: LIBS += -L$$OUT_PWD/../Core/ -lCore
