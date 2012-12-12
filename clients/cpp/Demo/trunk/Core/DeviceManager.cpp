#include "DeviceManager.h"

#include "CasparDevice.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

Q_GLOBAL_STATIC(DeviceManager, deviceManager)

DeviceManager::DeviceManager()
{
    this->device = new CasparDevice();
}

DeviceManager::~DeviceManager()
{
    if (this->device->isConnected())
        this->device->disconnect();

    delete this->device;
}

DeviceManager& DeviceManager::getInstance()
{
    return *deviceManager();
}

CasparDevice& DeviceManager::getDevice()
{
    return *this->device;
}
