#pragma once

#include "Shared.h"
#include "ui_SqueezeWidget.h"

#include <QtCore/QEvent>

#include <QtGui/QWidget>

class WIDGETS_EXPORT SqueezeWidget : public QWidget, Ui::SqueezeWidget
{
    Q_OBJECT

    public:
        explicit SqueezeWidget(QWidget* parent = 0);

    protected:
        bool eventFilter(QObject* target, QEvent* event);

    private:
};
