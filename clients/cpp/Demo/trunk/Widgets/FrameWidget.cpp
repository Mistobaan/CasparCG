#include "FrameWidget.h"

#include "DeviceManager.h"

#include <QtCore/QRegExp>

FrameWidget::FrameWidget(QWidget* parent) : QWidget(parent)
{
    setupUi(this);

    QObject::connect(&DeviceManager::getInstance().getDevice(), SIGNAL(responseChanged(const QList<QString>&, CasparDevice&)), this, SLOT(responseChanged(const QList<QString>&, CasparDevice&)));

    qApp->installEventFilter(this);
}

bool FrameWidget::eventFilter(QObject* target, QEvent* event)
{
    //if(event->type() == static_cast<QEvent::Type>(EventEnum::Statusbar))
    //{
    //    StatusbarEvent* statusbarEvent = static_cast<StatusbarEvent*>(event);
    //    statusBar()->showMessage(statusbarEvent->getMessage(), 5000);
    //}

    return QObject::eventFilter(target, event);
}

void FrameWidget::startDemo()
{
    this->plainTextEditResponse->clear();
    QStringList commands = this->plainTextEditCommands->toPlainText().split(QRegExp("\n"));
    foreach (const QString& command, commands)
    {
        if (command.isEmpty() || command.startsWith("//"))
            continue;

        DeviceManager::getInstance().getDevice().sendCommand(command);
    }
}

void FrameWidget::stopDemo()
{
    DeviceManager::getInstance().getDevice().clearChannel(1);
}

void FrameWidget::responseChanged(const QList<QString>& response, CasparDevice& device)
{
    foreach (const QString& value, response)
        this->plainTextEditResponse->appendPlainText(value);
}
