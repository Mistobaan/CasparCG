#include "BigFourWidget.h"

#include <QtCore/QDebug>
#include <QtCore/QString>

BigFourWidget::BigFourWidget(QWidget* parent) : QWidget(parent)
{
    setupUi(this);

    qApp->installEventFilter(this);
}

bool BigFourWidget::eventFilter(QObject* target, QEvent* event)
{
    //if(event->type() == static_cast<QEvent::Type>(EventEnum::Statusbar))
    //{
    //    StatusbarEvent* statusbarEvent = static_cast<StatusbarEvent*>(event);
    //    statusBar()->showMessage(statusbarEvent->getMessage(), 5000);
    //}

    return QObject::eventFilter(target, event);
}
