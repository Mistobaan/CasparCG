#pragma once

#include "Shared.h"

class CASPAR_EXPORT CasparData
{
    public:
        explicit CasparData(const QString& name);

        const QString& getName() const;

    private:
        QString name;
};
