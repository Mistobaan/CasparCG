#pragma once

#include "Shared.h"

class CASPAR_EXPORT CasparTemplate
{
    public:
        explicit CasparTemplate(const QString& name);

        const QString& getName() const;

    private:
        QString name;
};
