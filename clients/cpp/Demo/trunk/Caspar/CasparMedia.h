#pragma once

#include "Shared.h"

class CASPAR_EXPORT CasparMedia
{
    public:
        explicit CasparMedia(const QString& name, const QString& type);

        const QString& getName() const;
        const QString& getType() const;

    private:
        QString name;
        QString type;
};
