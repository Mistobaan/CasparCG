#pragma once

#include "Shared.h"

#include "CasparDevice.h"

class CORE_EXPORT DeviceManager
{
    public:
        explicit DeviceManager();
        ~DeviceManager();

        static DeviceManager& getInstance();

        CasparDevice& getDevice();

    private:
        CasparDevice* device;
};
