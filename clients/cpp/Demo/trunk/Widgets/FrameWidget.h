#pragma once

#include "Shared.h"
#include "ui_FrameWidget.h"

#include "CasparDevice.h"

#include <QtCore/QEvent>
#include <QtGui/QWidget>

class WIDGETS_EXPORT FrameWidget : public QWidget, Ui::FrameWidget
{
    Q_OBJECT

    public:
        explicit FrameWidget(QWidget* parent = 0);

    protected:
        bool eventFilter(QObject* target, QEvent* event);

    private:
        Q_SLOT void startDemo();
        Q_SLOT void stopDemo();
        Q_SLOT void responseChanged(const QList<QString>&, CasparDevice&);
};
