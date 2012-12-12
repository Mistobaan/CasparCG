#pragma once

#include "Shared.h"
#include "ICommand.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>

class CORE_EXPORT FileRecorderCommand : public QObject, public ICommand
{
    Q_OBJECT

    public:
        explicit FileRecorderCommand(QObject* parent = 0);

        virtual int getDelay() const;
        virtual int getChannel() const;
        virtual int getVideolayer() const;
        virtual bool getAllowGpi() const;

        virtual void setChannel(int channel);
        virtual void setVideolayer(int videolayer);
        virtual void setDelay(int delay);
        virtual void setAllowGpi(bool allowGpi);

        virtual void loadProperties(const boost::property_tree::wptree& pt);
        virtual void saveProperties(const boost::property_tree::wptree& pt);

        const QString& getFilename() const;
        const QString& getContainer() const;
        const QString& getCodec() const;
        const QString& getPreset() const;
        const QString& getTune() const;

        void setFilename(const QString& filename);
        void setContainer(const QString& container);
        void setCodec(const QString& codec);
        void setPreset(const QString& preset);
        void setTune(const QString& tune);

    private:
        int channel;
        int videolayer;
        int delay;
        bool allowGpi;
        QString filename;
        QString container;
        QString codec;
        QString preset;
        QString tune;

        Q_SIGNAL void allowGpiChanged(bool);
        Q_SIGNAL void channelChanged(int);
        Q_SIGNAL void videolayerChanged(int);
        Q_SIGNAL void delayChanged(int);
        Q_SIGNAL void filenameChanged(const QString&);
        Q_SIGNAL void containerChanged(const QString&);
        Q_SIGNAL void codecChanged(const QString&);
        Q_SIGNAL void presetChanged(const QString&);
        Q_SIGNAL void tuneChanged(const QString&);
};
