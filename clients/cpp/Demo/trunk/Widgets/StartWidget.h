#pragma once

#include "Shared.h"
#include "ui_StartWidget.h"

#include "CasparDevice.h"
#include "CasparVersion.h"

#include <QtCore/QEvent>

#include <QtGui/QWidget>

class WIDGETS_EXPORT StartWidget : public QWidget, Ui::StartWidget
{
    Q_OBJECT

    public:
        explicit StartWidget(QWidget* parent = 0);
        ~StartWidget();

    protected:
        bool eventFilter(QObject* target, QEvent* event);

    private:
        Q_SLOT void connectDevice();
        Q_SLOT void disconnectDevice();
        Q_SLOT void deviceConnectionStateChanged(CasparDevice&);
        Q_SLOT void deviceVersionChanged(const CasparVersion&, CasparDevice&);
};
