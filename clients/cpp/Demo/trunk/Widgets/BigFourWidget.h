#pragma once

#include "Shared.h"
#include "ui_BigFourWidget.h"

#include <QtCore/QEvent>

#include <QtGui/QWidget>

class WIDGETS_EXPORT BigFourWidget : public QWidget, Ui::BigFourWidget
{
    Q_OBJECT

    public:
        explicit BigFourWidget(QWidget* parent = 0);

    protected:
        bool eventFilter(QObject* target, QEvent* event);

    private:
};
