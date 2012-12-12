#-------------------------------------------------
#
# Project created by QtCreator 2011-04-07T13:50:44
#
#-------------------------------------------------

QT += core gui sql

TARGET = widgets
TEMPLATE = lib

DEFINES += WIDGETS_LIBRARY

HEADERS += \
    SettingsDialog.h \
    MainWindow.h \
    ClockWidget.h \
    PreviewWidget.h \
    AboutDialog.h \
    AddDeviceDialog.h \
    Shared.h \
    TimelineWidget.h \
    HelpDialog.h \
    Inspector/InspectorMediaWidget.h \
    Inspector/InspectorWidget.h \
    Inspector/InspectorTemplateWidget.h \
    Inspector/InspectorOutputWidget.h \
    Inspector/InspectorMetadataWidget.h \
    Inspector/InspectorLevelsWidget.h \
    Inspector/InspectorGeometryWidget.h \
    Library/LibraryWidget.h \
    Rundown/RundownGroupWidget.h \
    Rundown/RundownMediaWidget.h \
    Rundown/RundownTemplateWidget.h \
    Rundown/RundownWidget.h \
    Rundown/IRundownWidget.h \
    Rundown/IPlayoutCommand.h \
    Rundown/RundownCropWidget.h \
    Inspector/InspectorCropWidget.h \
    Rundown/RundownGeometryWidget.h \
    Inspector/InspectorBrightnessWidget.h \
    Rundown/RundownBrightnessWidget.h \
    Inspector/InspectorSaturationWidget.h \
    Rundown/RundownSaturationWidget.h \
    Rundown/RundownOpacityWidget.h \
    Inspector/InspectorOpacityWidget.h \
    Rundown/RundownContrastWidget.h \
    Inspector/InspectorContrastWidget.h \
    Rundown/RundownVolumeWidget.h \
    Inspector/InspectorVolumeWidget.h \
    Rundown/RundownLevelsWidget.h \
    Rundown/RundownKeyerWidget.h \
    Rundown/RundownBlendWidget.h \
    Rundown/RundownGridWidget.h \
    Inspector/InspectorGridWidget.h \
    Rundown/RundownGpiOutputWidget.h \
    Rundown/RundownDeckLinkInputWidget.h \
    Inspector/InspectorDeckLinkInputWidget.h \
    Rundown/RundownCommitWidget.h \
    Inspector/InspectorGpiOutputWidget.h \
    Rundown/RundownImageScrollerWidget.h \
    Inspector/InspectorImageScrollerWidget.h \
    Rundown/RundownFileRecorderWidget.h \
    Inspector/InspectorFileRecorderWidget.h \
    Inspector/InspectorBlendModeWidget.h

SOURCES += \
    SettingsDialog.cpp \
    ClockWidget.cpp \
    PreviewWidget.cpp \
    AboutDialog.cpp \
    AddDeviceDialog.cpp \
    MainWindow.cpp \
    TimelineWidget.cpp \
    HelpDialog.cpp \
    Inspector/InspectorMediaWidget.cpp \
    Inspector/InspectorWidget.cpp \
    Inspector/InspectorTemplateWidget.cpp \
    Inspector/InspectorOutputWidget.cpp \
    Inspector/InspectorMetadataWidget.cpp \
    Inspector/InspectorLevelsWidget.cpp \
    Inspector/InspectorGeometryWidget.cpp \
    Inspector/InspectorImageScrollerWidget.cpp \
    Inspector/InspectorCropWidget.cpp \
    Inspector/InspectorBrightnessWidget.cpp \
    Inspector/InspectorOpacityWidget.cpp \
    Inspector/InspectorSaturationWidget.cpp \
    Inspector/InspectorContrastWidget.cpp \
    Inspector/InspectorVolumeWidget.cpp \
    Inspector/InspectorGridWidget.cpp \
    Inspector/InspectorDeckLinkInputWidget.cpp \
    Inspector/InspectorGpiOutputWidget.cpp \
    Inspector/InspectorFileRecorderWidget.cpp \
    Library/LibraryWidget.cpp \
    Rundown/RundownCropWidget.cpp \
    Rundown/RundownGroupWidget.cpp \
    Rundown/RundownMediaWidget.cpp \
    Rundown/RundownTemplateWidget.cpp \
    Rundown/RundownWidget.cpp \
    Rundown/RundownGeometryWidget.cpp \
    Rundown/RundownBrightnessWidget.cpp \
    Rundown/RundownSaturationWidget.cpp \
    Rundown/RundownOpacityWidget.cpp \
    Rundown/RundownContrastWidget.cpp \
    Rundown/RundownVolumeWidget.cpp \
    Rundown/RundownLevelsWidget.cpp \
    Rundown/RundownKeyerWidget.cpp \
    Rundown/RundownBlendWidget.cpp \
    Rundown/RundownGridWidget.cpp \
    Rundown/RundownGpiOutputWidget.cpp \
    Rundown/RundownDeckLinkInputWidget.cpp \
    Rundown/RundownCommitWidget.cpp \
    Rundown/RundownImageScrollerWidget.cpp \
    Rundown/RundownFileRecorderWidget.cpp \
    Inspector/InspectorBlendModeWidget.cpp

FORMS += \
    SettingsDialog.ui \
    MainWindow.ui \
    ClockWidget.ui \
    PreviewWidget.ui \
    AboutDialog.ui \
    AddDeviceDialog.ui \
    TimelineWidget.ui \
    HelpDialog.ui \
    Inspector/InspectorMediaWidget.ui \
    Inspector/InspectorVolumeWidget.ui \
    Inspector/InspectorWidget.ui \
    Inspector/InspectorTemplateWidget.ui \
    Inspector/InspectorOutputWidget.ui \
    Inspector/InspectorMetadataWidget.ui \
    Inspector/InspectorGeometryWidget.ui \
    Inspector/InspectorGridWidget.ui \
    Inspector/InspectorDeckLinkInputWidget.ui \
    Inspector/InspectorGpiOutputWidget.ui \
    Inspector/InspectorImageScrollerWidget.ui \
    Inspector/InspectorFileRecorderWidget.ui \
    Library/LibraryWidget.ui \
    Rundown/RundownWidget.ui \
    Rundown/RundownTemplateWidget.ui \
    Rundown/RundownMediaWidget.ui \
    Rundown/RundownGroupWidget.ui \
    Inspector/InspectorLevelsWidget.ui \
    Inspector/InspectorCropWidget.ui \
    Rundown/RundownCropWidget.ui \
    Inspector/InspectorBrightnessWidget.ui \
    Rundown/RundownGeometryWidget.ui \
    Rundown/RundownBrightnessWidget.ui \
    Inspector/InspectorSaturationWidget.ui \
    Rundown/RundownSaturationWidget.ui \
    Rundown/RundownOpacityWidget.ui \
    Inspector/InspectorOpacityWidget.ui \
    Rundown/RundownContrastWidget.ui \
    Inspector/InspectorContrastWidget.ui \
    Rundown/RundownVolumeWidget.ui \
    Rundown/RundownLevelsWidget.ui \
    Rundown/RundownKeyerWidget.ui \
    Rundown/RundownBlendWidget.ui \
    Rundown/RundownGridWidget.ui \ 
    Rundown/RundownGpiOutputWidget.ui \
    Rundown/RundownDeckLinkInputWidget.ui \ 
    Rundown/RundownCommitWidget.ui \
    Rundown/RundownImageScrollerWidget.ui \ 
    Rundown/RundownFileRecorderWidget.ui \
    Inspector/InspectorBlendModeWidget.ui

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
    Images/Logo.png \
    Images/CasparCG.png \
    Images/ArrowLeftDisabled.png \
    Images/ArrowLeft.png \
    Images/ArrowRightDisabled.png \
    Images/ArrowRight.png \
    Images/Forward.png \
    Images/FastForwardEnd.png \
    Images/Rewind.png \
    Images/FastRewindStart.png \
    Images/Splitter.png \
    Images/Play.png \
    Images/FastForward.png \
    Images/RewindStart.png \
    Images/FastRewind.png \
    Images/Stop.png \
    Images/Pause.png \
    Images/Recording.png \
    Images/Record.png \
    Images/GreenBullet.png \
    Images/YellowBullet.png \
    Images/RedBullet.png \
    Images/BlueBullet.png \
    Images/CheckboxChecked.png \
    Images/Refresh.png \
    Images/Group.png \
    Images/GreyBullet.png \
    Images/Audio.png \
    Images/Mixer.png \
    Images/Template.png \
    Images/Recorder.png \
    Images/Producer.png \
    Images/Preview.png \
    Images/Movie.png \
    Images/Still.png \
    Images/Consumer.png \
    Images/Disconnected.png \
    Images/Gpi.png \
    Images/GpiTriggerable.png \
    Images/GpiTriggerableDisconnected.png \
    Images/Ungroup.png \
    Images/New.png \
    Images/Color.png \
    Fonts/OpenSans-SemiboldItalic.ttf \
    Fonts/OpenSans-Semibold.ttf \
    Fonts/OpenSans-Regular.ttf \
    Fonts/OpenSans-LightItalic.ttf \
    Fonts/OpenSans-Light.ttf \
    Fonts/OpenSans-Italic.ttf \
    Fonts/OpenSans-ExtraBoldItalic.ttf \
    Fonts/OpenSans-ExtraBold.ttf \
    Fonts/OpenSans-BoldItalic.ttf \
    Fonts/OpenSans-Bold.ttf \
    Images/ImageScroller.png

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

DEPENDPATH += $$PWD/../../dependencies/gpio-client/include
INCLUDEPATH += $$PWD/../../dependencies/gpio-client/include
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../dependencies/gpio-client/lib/win32/release/ -lgpio-client
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../dependencies/gpio-client/lib/win32/debug/ -lgpio-client
