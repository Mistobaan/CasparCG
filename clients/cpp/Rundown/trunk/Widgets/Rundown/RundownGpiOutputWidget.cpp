#include "RundownGpiOutputWidget.h"

#include "Global.h"

#include "DeviceManager.h"
#include "GpiManager.h"
#include "Events/ConnectionStateChangedEvent.h"
#include "Events/RundownItemChangedEvent.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>

RundownGpiOutputWidget::RundownGpiOutputWidget(const LibraryModel& model, QWidget* parent, const QString& color,
                                               bool active, bool inGroup)
    : QWidget(parent),
      active(active), inGroup(inGroup), color(color), model(model)
{
    setupUi(this);

    setActive(active);

    this->labelDisconnected->setVisible(!GpiManager::getInstance().getGpiDevice()->isConnected());
    this->labelGroupColor->setVisible(this->inGroup);
    this->labelGroupColor->setStyleSheet(QString("background-color: %1;").arg(Color::DEFAULT_GROUP_COLOR));
    this->labelColor->setStyleSheet(QString("background-color: %1;").arg(color));

    this->labelName->setText(this->model.getName());
    this->labelDelay->setText(QString("Delay: %1").arg(this->command.getDelay()));
    gpiOutputPortChanged(this->command.getGpoPort());

    QObject::connect(&this->command, SIGNAL(delayChanged(int)), this, SLOT(delayChanged(int)));
    QObject::connect(&this->command, SIGNAL(gpoPortChanged(int)), this, SLOT(gpiOutputPortChanged(int)));
    QObject::connect(&this->command, SIGNAL(allowGpiChanged(bool)), this, SLOT(allowGpiChanged(bool)));
    QObject::connect(GpiManager::getInstance().getGpiDevice().data(), SIGNAL(connectionStateChanged(bool, GpiDevice*)),
                     this, SLOT(gpiDeviceConnected(bool, GpiDevice*)));

    checkGpiTriggerable();

    qApp->installEventFilter(this);
}

bool RundownGpiOutputWidget::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == static_cast<QEvent::Type>(Enum::EventType::RundownItemChanged))
    {
        // This event is not for us.
        if (!this->active)
            return false;

        RundownItemChangedEvent* rundownItemChangedEvent = dynamic_cast<RundownItemChangedEvent*>(event);
        this->model.setName(rundownItemChangedEvent->getName());

        this->labelName->setText(this->model.getName());
    }
    else if (event->type() == static_cast<QEvent::Type>(Enum::EventType::RundownItemPreview))
    {
        // This event is not for us.
        if (!this->active)
            return false;

        executePlay();
    }

    return QObject::eventFilter(target, event);
}

IRundownWidget* RundownGpiOutputWidget::clone()
{
    RundownGpiOutputWidget* widget = new RundownGpiOutputWidget(this->model, this->parentWidget(), this->color,
                                                                this->active, this->inGroup);

    GpiOutputCommand* command = dynamic_cast<GpiOutputCommand*>(widget->getCommand());
    command->setDelay(this->command.getDelay());
    command->setAllowGpi(this->command.getAllowGpi());
    command->setGpoPort(this->command.getGpoPort());

    return widget;
}

bool RundownGpiOutputWidget::isGroup() const
{
    return false;
}

ICommand* RundownGpiOutputWidget::getCommand()
{
    return &this->command;
}

LibraryModel* RundownGpiOutputWidget::getLibraryModel()
{
    return &this->model;
}

void RundownGpiOutputWidget::setActive(bool active)
{
    this->active = active;

    if (this->active)
        this->labelActiveColor->setStyleSheet("background-color: red;");
    else
        this->labelActiveColor->setStyleSheet("");
}

void RundownGpiOutputWidget::setInGroup(bool inGroup)
{
    this->inGroup = inGroup;
    this->labelGroupColor->setVisible(inGroup);
}

void RundownGpiOutputWidget::setColor(const QString& color)
{
    this->color = color;
    this->labelColor->setStyleSheet(QString("background-color: %1;").arg(color));
}

bool RundownGpiOutputWidget::executeCommand(enum Playout::PlayoutType::Type type)
{
    if (type == Playout::PlayoutType::Play)
        QTimer::singleShot(this->command.getDelay(), this, SLOT(executePlay()));

    return true;
}

void RundownGpiOutputWidget::executePlay()
{
    GpiManager::getInstance().getGpiDevice()->trigger(command.getGpoPort());
}

void RundownGpiOutputWidget::delayChanged(int delay)
{
    this->labelDelay->setText(QString("Delay: %1").arg(delay));
}

void RundownGpiOutputWidget::gpiOutputPortChanged(int port)
{
    this->labelGpiOutputPort->setText(QString("GPO Port: %1").arg(port + 1));
}

void RundownGpiOutputWidget::checkGpiTriggerable()
{
    labelGpiTriggerable->setVisible(this->command.getAllowGpi());

    if (GpiManager::getInstance().getGpiDevice()->isConnected())
        labelGpiTriggerable->setPixmap(QPixmap(":/Graphics/Images/GpiTriggerable.png"));
    else
        labelGpiTriggerable->setPixmap(QPixmap(":/Graphics/Images/GpiTriggerableDisconnected.png"));
}

void RundownGpiOutputWidget::allowGpiChanged(bool allowGpi)
{
    checkGpiTriggerable();
}


void RundownGpiOutputWidget::gpiDeviceConnected(bool connected, GpiDevice* device)
{
    this->labelDisconnected->setVisible(!connected);
    checkGpiTriggerable();
}
