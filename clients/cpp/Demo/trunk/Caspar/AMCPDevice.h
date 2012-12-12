#pragma once

#include "Shared.h"

#include <QtCore/QObject>

#include <QtNetwork/QTcpSocket>

class CASPAR_EXPORT AMCPDevice : public QObject
{
    Q_OBJECT

    public:
        explicit AMCPDevice(QObject* parent = 0);
        ~AMCPDevice();

        bool isConnected() const;

    protected:
        enum AMCPCommand
        {
            NONE,
            CONNECTIONSTATE,
            LOAD,
            LOADBG,
            PLAY,
            STOP,
            CG,
            CLS,
            CINF,
            VERSION,
            TLS,
            INFO,
            INFOSYSTEM,
            DATALIST,
            CLEAR,
            SET,
            MIXER,
            CALL,
            REMOVE,
            ADD,
            SWAP,
            STATUS
        };

        QTcpSocket* socket;
        AMCPCommand command;

        QList<QString> response;

        virtual void sendNotification() = 0;

        void connectDevice(const QString& ip, int port);
        void disconnectDevice();

        void resetDevice();
        void writeMessage(const QString& message);

    private:
        enum AMCPParserState
        {
            ExpectingHeader,
            ExpectingOneline,
            ExpectingTwoline,
            ExpectingMultiline
        };

        int code;
        int state;
        bool connected;
        QString line;
        QString previousLine;

        void parseLine(const QString& line);
        void parseHeader(const QString& line);
        void parseOneline(const QString& line);
        void parseTwoline(const QString& line);
        void parseMultiline(const QString& line);

        AMCPCommand translateCommand(const QString& command);

        Q_SLOT void readMessage();
        Q_SLOT void setConnected();
        Q_SLOT void setDisconnected();
};
