#pragma once

#pragma GCC diagnostic ignored "-Wunused-variable"

#define MAJOR_VERSION 2
#define MINOR_VERSION 0
#define TAG_VERSION "Beta 1"
#define DATABASE_VERSION "20b1"

#include <stdexcept>

#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QtCore/QList>

namespace Enum
{
    struct EventType
    {
        enum Type
        {
            StatusbarMessage = QEvent::User + 11,
            LibraryItemSelected = QEvent::User + 12,
            RundownItemSelected = QEvent::User + 13,
            MediaChanged = QEvent::User + 14,
            TemplateChanged = QEvent::User + 15,
            DataChanged = QEvent::User + 16,
            Synchronize = QEvent::User + 17,
            AutoSynchronize = QEvent::User + 18,
            AddRudnownItem = QEvent::User + 19,
            RundownItemChanged = QEvent::User + 20,
            RundownItemPreview = QEvent::User + 21,
            ConnectionStateChanged = QEvent::User + 22,
            RundownIsEmpty = QEvent::User + 23,
            OpenRundown = QEvent::User + 24
        };
    };
}

namespace Playout
{
    struct PlayoutType
    {
        enum Type
        {
            Stop = QEvent::User + 110,
            Play = QEvent::User + 120,
            Pause = QEvent::User + 130,
            Load = QEvent::User + 140,
            Next = QEvent::User + 150,
            Update = QEvent::User + 160,
            Invoke = QEvent::User + 170,
            Clear = QEvent::User + 180,
            ClearVideolayer = QEvent::User + 190,
            ClearChannel = QEvent::User + 200
        };
    };

    PlayoutType::Type fromString(const QString& value);
    QString toString(PlayoutType::Type value);
    const QList<PlayoutType::Type>& enumConstants();
}

namespace Gpi
{
    static const int DEFAULT_GPO_PORT = 0;
}

namespace Mixer
{
    static const float DEFAULT_GRID = 2;
    static const float DEFAULT_BRIGHTNESS = 1;
    static const float DEFAULT_CONTRAST = 1;
    static const float DEFAULT_CROP_LEFT = 0;
    static const float DEFAULT_CROP_RIGHT = 0;
    static const float DEFAULT_CROP_TOP = 1;
    static const float DEFAULT_CROP_BOTTOM = 1;
    static const float DEFAULT_GEOMETRY_POSITION_X = 0;
    static const float DEFAULT_GEOMETRY_POSITION_Y = 0;
    static const float DEFAULT_GEOMETRY_SCALE_X = 1;
    static const float DEFAULT_GEOMETRY_SCALE_Y = 1;
    static const float DEFAULT_LEVELS_MIN_IN = 0;
    static const float DEFAULT_LEVELS_MAX_IN = 1;
    static const float DEFAULT_LEVELS_MIN_OUT = 0;
    static const float DEFAULT_LEVELS_MAX_OUT = 1;
    static const float DEFAULT_LEVELS_GAMMA = 1;
    static const float DEFAULT_OPACITY = 1;
    static const float DEFAULT_SATURATION = 1;
    static const float DEFAULT_VOLUME = 1;
    static const int DEFAULT_DURATION = 12;
    static const bool DEFAULT_DEFER = false;
    static const QString DEFAULT_BLENDMODE = "Normal";
    static const QString DEFAULT_TWEEN = "Linear";
    static const QString DEFAULT_DIRECTION = "RIGHT";
    static const QString DEFAULT_TRANSITION = "CUT";
}

namespace Color
{
    static const QString DEFAULT_CONSUMER_COLOR = "DarkSlateGray";
    static const QString DEFAULT_GPI_COLOR = "Chocolate";
    static const QString DEFAULT_GROUP_COLOR = "SteelBlue";
    static const QString DEFAULT_MEDIA_COLOR = "Goldenrod";
    static const QString DEFAULT_MIXER_COLOR = "Sienna";
    static const QString DEFAULT_PRODUCER_COLOR = "SeaGreen";
    static const QString DEFAULT_TEMPLATE_COLOR = "OliveDrab";
}

namespace TemplateData
{
    static const QString DEFAULT_COMPONENT_DATA = "<componentData id=\\\"#KEY\\\"><data id=\\\"text\\\" value=\\\"#VALUE\\\"/></componentData>";
}

namespace Output
{
    static const int DEFAULT_CHANNEL = 1;
    static const int DEFAULT_VIDEOLAYER = 10;
    static const int DEFAULT_VIDEOLAYER_FLASH = 10;
    static const int DEFAULT_DELAY = 0;
    static const bool DEFAULT_ALLOW_GPI = false;
}

namespace Media
{
    static const QString DEFAULT_MEDIA_NAME = "";
    static const int DEFAULT_SEEK = 0;
    static const int DEFAULT_LENGTH = 0;
    static const int DEFAULT_BLUR = 0;
    static const int DEFAULT_SPEED = 8;
    static const bool DEFAULT_LOOP = false;
    static const bool DEFAULT_PREMULTIPLY = false;
    static const bool DEFAULT_PROGRESSIVE = false;
}

namespace Template
{
    static const int DEFAULT_FLASHLAYER = 1;
    static const QString DEFAULT_INVOKE = "";
    static const QString DEFAULT_TEMPLATENAME = "";
}

namespace DeckLinkInput
{
    static const int DEFAULT_DEVICE = 2;
    static const QString DEFAULT_FORMAT = "PAL";
}

namespace FileRecorder
{
    static const QString DEFAULT_OUTPUT = "Output.mov";
    static const QString DEFAULT_CONTAINER = "mov";
    static const QString DEFAULT_CODEC = "qtrle";
    static const QString DEFAULT_PRESET = "ultrafast";
    static const QString DEFAULT_TUNE = "fastdecode";
}
