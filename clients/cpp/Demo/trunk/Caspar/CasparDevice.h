#pragma once

#include "Shared.h"

#include "AMCPDevice.h"
#include "CasparData.h"
#include "CasparMedia.h"
#include "CasparTemplate.h"
#include "CasparVersion.h"

class CASPAR_EXPORT CasparDevice : public AMCPDevice
{
    Q_OBJECT

    public:
        explicit CasparDevice(QObject* parent = 0);

        void connect(const QString& name, int port = 5250);
        void disconnect();

        int getPort() const;
        QString getName() const;

        void refreshData();
        void refreshMedia();
        void refreshTemplate();
        void refreshVersion();

        void clearChannel(int channel);
        void clearVideolayer(int channel, int videolayer);

        void sendCommand(const QString& command);

        void addTemplate(int channel, int flashlayer, const QString& name, int playOnLoad, const QString& data);
        void addTemplate(int channel, int videolayer, int flashlayer, const QString& name, int playOnLoad, const QString& data);

        void playTemplate(int channel, int flashlayer);
        void playTemplate(int channel, int flashlayer, const QString& item);
        void playTemplate(int channel, int videolayer, int flashlayer);
        void playTemplate(int channel, int videolayer, int flashlayer, const QString& item);

        void stopTemplate(int channel, int flashlayer);
        void stopTemplate(int channel, int videolayer, int flashlayer);      

        void playMedia(int channel, const QString& item);
        void playMedia(int channel, int videolayer, const QString& item);

        void stopMedia(int channel);
        void stopMedia(int channel, int videolayer);

        void startRecording(int channel, const QString& filename);
        void startRecording(int channel, const QString& filename, const QString& params);
        void startRecording(int channel, int videolayer, const QString& filename);
        void startRecording(int channel, int videolayer, const QString& filename, const QString& params);

        void stopRecording(int channel);
        void stopRecording(int channel, int videolayer);

        void setVolume(int channel, float volume);
        void setVolume(int channel, float volume, int duration, QString easing);
        void setVolume(int channel, int videolayer, float volume);
        void setVolume(int channel, int videolayer, float volume, int duration, QString easing);

        void setOpacity(int channel, float opacity);
        void setOpacity(int channel, float opacity, int duration, QString easing);
        void setOpacity(int channel, int videolayer, float opacity);
        void setOpacity(int channel, int videolayer, float opacity, int duration, QString easing);

        void setBrightness(int channel, float brightness);
        void setBrightness(int channel, float brightness, int duration, QString easing);
        void setBrightness(int channel, int videolayer, float brightness);
        void setBrightness(int channel, int videolayer, float brightness, int duration, QString easing);

        void setContrast(int channel, float contrast);
        void setContrast(int channel, float contrast, int duration, QString easing);
        void setContrast(int channel, int videolayer, float contrast);
        void setContrast(int channel, int videolayer, float contrast, int duration, QString easing);

        void setSaturation(int channel, float saturation);
        void setSaturation(int channel, float saturation, int duration, QString easing);
        void setSaturation(int channel, int videolayer, float saturation);
        void setSaturation(int channel, int videolayer, float saturation, int duration, QString easing);

        void setLevels(int channel, float minIn, float maxIn, float gamma, float minOut, float maxOut);
        void setLevels(int channel, float minIn, float maxIn, float gamma, float minOut, float maxOut, int duration, QString easing);
        void setLevels(int channel, int videolayer, float minIn, float maxIn, float gamma, float minOut, float maxOut);
        void setLevels(int channel, int videolayer, float minIn, float maxIn, float gamma, float minOut, float maxOut, int duration, QString easing);

        void setGeometry(int channel, float positionX, float positionY, float scaleX, float scaleY);
        void setGeometry(int channel, float positionX, float positionY, float scaleX, float scaleY, int duration, QString easing);
        void setGeometry(int channel, int videolayer, float positionX, float positionY, float scaleX, float scaleY);
        void setGeometry(int channel, int videolayer, float positionX, float positionY, float scaleX, float scaleY, int duration, QString easing);

        void setClipping(int channel, float positionX, float positionY, float scaleX, float scaleY);
        void setClipping(int channel, float positionX, float positionY, float scaleX, float scaleY, int duration, QString easing);
        void setClipping(int channel, int videolayer, float positionX, float positionY, float scaleX, float scaleY);
        void setClipping(int channel, int videolayer, float positionX, float positionY, float scaleX, float scaleY, int duration, QString easing);

        Q_SIGNAL void connectionStateChanged(CasparDevice&);
        Q_SIGNAL void infoChanged(const QList<QString>&, CasparDevice&);
        Q_SIGNAL void infoSystemChanged(const QList<QString>&, CasparDevice&);
        Q_SIGNAL void mediaChanged(const QList<CasparMedia>&, CasparDevice&);
        Q_SIGNAL void mediaInfoChanged(const QList<QString>&, CasparDevice&);
        Q_SIGNAL void templateChanged(const QList<CasparTemplate>&, CasparDevice&);
        Q_SIGNAL void dataChanged(const QList<CasparData>&, CasparDevice&);
        Q_SIGNAL void versionChanged(const CasparVersion&, CasparDevice&);
        Q_SIGNAL void responseChanged(const QList<QString>&, CasparDevice&);

    protected:
        void sendNotification();
};
