#include "RundownGeometryWidget.h"

#include "Global.h"

#include "DeviceManager.h"
#include "GpiManager.h"
#include "Events/ConnectionStateChangedEvent.h"
#include "Events/RundownItemChangedEvent.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>

RundownGeometryWidget::RundownGeometryWidget(const LibraryModel& model, QWidget* parent, const QString& color,
                                             bool active, bool inGroup, bool disconnected)
    : QWidget(parent),
      active(active), inGroup(inGroup), disconnected(disconnected), color(color), model(model)
{
    setupUi(this);

    setActive(active);

    this->labelDisconnected->setVisible(this->disconnected);
    this->labelGroupColor->setVisible(this->inGroup);
    this->labelGroupColor->setStyleSheet(QString("background-color: %1;").arg(Color::DEFAULT_GROUP_COLOR));
    this->labelColor->setStyleSheet(QString("background-color: %1;").arg(color));

    this->labelName->setText(this->model.getName());
    this->labelChannel->setText(QString("Channel: %1").arg(this->command.getChannel()));
    this->labelVideolayer->setText(QString("Videolayer: %1").arg(this->command.getVideolayer()));
    this->labelDelay->setText(QString("Delay: %1").arg(this->command.getDelay()));
    this->labelDevice->setText(QString("Device: %1").arg(this->model.getDeviceName()));

    QObject::connect(&this->command, SIGNAL(channelChanged(int)), this, SLOT(channelChanged(int)));
    QObject::connect(&this->command, SIGNAL(videolayerChanged(int)), this, SLOT(videolayerChanged(int)));
    QObject::connect(&this->command, SIGNAL(delayChanged(int)), this, SLOT(delayChanged(int)));
    QObject::connect(&this->command, SIGNAL(allowGpiChanged(bool)), this, SLOT(allowGpiChanged(bool)));
    QObject::connect(GpiManager::getInstance().getGpiDevice().data(), SIGNAL(connectionStateChanged(bool, GpiDevice*)),
                     this, SLOT(gpiDeviceConnected(bool, GpiDevice*)));

    checkEmptyDevice();
    checkGpiTriggerable();

    qApp->installEventFilter(this);
}

bool RundownGeometryWidget::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == static_cast<QEvent::Type>(Enum::EventType::RundownItemChanged))
    {
        // This event is not for us.
        if (!this->active)
            return false;

        RundownItemChangedEvent* rundownItemChangedEvent = dynamic_cast<RundownItemChangedEvent*>(event);
        this->model.setName(rundownItemChangedEvent->getName());
        this->model.setDeviceName(rundownItemChangedEvent->getDeviceName());

        this->labelName->setText(this->model.getName());
        this->labelDevice->setText(QString("Device: %1").arg(this->model.getDeviceName()));

        checkEmptyDevice();
    }
    else if (event->type() == static_cast<QEvent::Type>(Enum::EventType::RundownItemPreview))
    {
        // This event is not for us.
        if (!this->active)
            return false;

        executePlay();
    }
    else if (event->type() == static_cast<QEvent::Type>(Enum::EventType::ConnectionStateChanged))
    {
        ConnectionStateChangedEvent* connectionStateChangedEvent = dynamic_cast<ConnectionStateChangedEvent*>(event);
        if (connectionStateChangedEvent->getDeviceName() == this->model.getDeviceName())
        {
            this->disconnected = !connectionStateChangedEvent->getConnected();

            if (connectionStateChangedEvent->getConnected())
                this->labelDisconnected->setVisible(false);
            else
                this->labelDisconnected->setVisible(true);
        }
    }

    return QObject::eventFilter(target, event);
}

IRundownWidget* RundownGeometryWidget::clone()
{
    RundownGeometryWidget* widget = new RundownGeometryWidget(this->model, this->parentWidget(), this->color,
                                                              this->active, this->inGroup, this->disconnected);

    GeometryCommand* command = dynamic_cast<GeometryCommand*>(widget->getCommand());
    command->setChannel(this->command.getChannel());
    command->setVideolayer(this->command.getVideolayer());
    command->setDelay(this->command.getDelay());
    command->setAllowGpi(this->command.getAllowGpi());
    command->setPositionX(this->command.getPositionX());
    command->setPositionY(this->command.getPositionY());
    command->setScaleX(this->command.getScaleX());
    command->setScaleY(this->command.getScaleY());
    command->setDuration(this->command.getDuration());
    command->setTween(this->command.getTween());
    command->setDefer(this->command.getDefer());

    return widget;
}

bool RundownGeometryWidget::isGroup() const
{
    return false;
}

ICommand* RundownGeometryWidget::getCommand()
{
    return &this->command;
}

LibraryModel* RundownGeometryWidget::getLibraryModel()
{
    return &this->model;
}

void RundownGeometryWidget::setActive(bool active)
{
    this->active = active;

    if (this->active)
        this->labelActiveColor->setStyleSheet("background-color: red;");
    else
        this->labelActiveColor->setStyleSheet("");
}

void RundownGeometryWidget::setInGroup(bool inGroup)
{
    this->inGroup = inGroup;
    this->labelGroupColor->setVisible(inGroup);
}

void RundownGeometryWidget::setColor(const QString& color)
{
    this->color = color;
    this->labelColor->setStyleSheet(QString("background-color: %1;").arg(color));
}

void RundownGeometryWidget::checkEmptyDevice()
{
    if (this->labelDevice->text() == "Device: ")
        this->labelDevice->setStyleSheet("color: black;");
    else
        this->labelDevice->setStyleSheet("");
}

bool RundownGeometryWidget::executeCommand(enum Playout::PlayoutType::Type type)
{
    if (type == Playout::PlayoutType::Stop)
        QTimer::singleShot(0, this, SLOT(executeStop()));
    else if (type == Playout::PlayoutType::Play)
        QTimer::singleShot(this->command.getDelay(), this, SLOT(executePlay()));
    else if (type == Playout::PlayoutType::Clear)
        QTimer::singleShot(0, this, SLOT(executeClear()));
    else if (type == Playout::PlayoutType::ClearVideolayer)
        QTimer::singleShot(0, this, SLOT(executeClearVideolayer()));
    else if (type == Playout::PlayoutType::ClearChannel)
        QTimer::singleShot(0, this, SLOT(executeClearChannel()));

    return true;
}

void RundownGeometryWidget::executeStop()
{
    const QSharedPointer<CasparDevice> device = DeviceManager::getInstance().getConnectionByName(this->model.getDeviceName());
    if (device != NULL && device->isConnected())
        device->setGeometry(this->command.getChannel(), this->command.getVideolayer(), 0, 0, 1, 1);

    foreach (const DeviceModel& model, DeviceManager::getInstance().getDeviceModels())
    {
        if (model.getShadow() == "No")
            continue;

        const QSharedPointer<CasparDevice> deviceShadow = DeviceManager::getInstance().getConnectionByName(model.getName());
        if (deviceShadow != NULL && deviceShadow->isConnected())
            deviceShadow->setGeometry(this->command.getChannel(), this->command.getVideolayer(), 0, 0, 1, 1);
    }
}

void RundownGeometryWidget::executePlay()
{
    const QSharedPointer<CasparDevice> device = DeviceManager::getInstance().getConnectionByName(this->model.getDeviceName());
    if (device != NULL && device->isConnected())
        device->setGeometry(this->command.getChannel(), this->command.getVideolayer(), this->command.getPositionX(),
                            this->command.getPositionY(), this->command.getScaleX(), this->command.getScaleY(),
                            this->command.getDuration(), this->command.getTween(), this->command.getDefer());

    foreach (const DeviceModel& model, DeviceManager::getInstance().getDeviceModels())
    {
        if (model.getShadow() == "No")
            continue;

        const QSharedPointer<CasparDevice>  deviceShadow = DeviceManager::getInstance().getConnectionByName(model.getName());
        if (deviceShadow != NULL && deviceShadow->isConnected())
            deviceShadow->setGeometry(this->command.getChannel(), this->command.getVideolayer(), this->command.getPositionX(),
                                      this->command.getPositionY(), this->command.getScaleX(), this->command.getScaleY(),
                                      this->command.getDuration(), this->command.getTween(), this->command.getDefer());
    }
}

void RundownGeometryWidget::executeClear()
{
    const QSharedPointer<CasparDevice> device = DeviceManager::getInstance().getConnectionByName(this->model.getDeviceName());
    if (device != NULL && device->isConnected())
        device->setGeometry(this->command.getChannel(), this->command.getVideolayer(), 0, 0, 1, 1);

    foreach (const DeviceModel& model, DeviceManager::getInstance().getDeviceModels())
    {
        if (model.getShadow() == "No")
            continue;

        const QSharedPointer<CasparDevice> deviceShadow = DeviceManager::getInstance().getConnectionByName(model.getName());
        if (deviceShadow != NULL && deviceShadow->isConnected())
            deviceShadow->setGeometry(this->command.getChannel(), this->command.getVideolayer(), 0, 0, 1, 1);
    }
}

void RundownGeometryWidget::executeClearVideolayer()
{
    const QSharedPointer<CasparDevice> device = DeviceManager::getInstance().getConnectionByName(this->model.getDeviceName());
    if (device != NULL && device->isConnected())
        device->clearMixerVideolayer(this->command.getChannel(), this->command.getVideolayer());

    foreach (const DeviceModel& model, DeviceManager::getInstance().getDeviceModels())
    {
        if (model.getShadow() == "No")
            continue;

        const QSharedPointer<CasparDevice> deviceShadow = DeviceManager::getInstance().getConnectionByName(model.getName());
        if (deviceShadow != NULL && deviceShadow->isConnected())
            deviceShadow->clearMixerVideolayer(this->command.getChannel(), this->command.getVideolayer());
    }
}

void RundownGeometryWidget::executeClearChannel()
{
    const QSharedPointer<CasparDevice> device = DeviceManager::getInstance().getConnectionByName(this->model.getDeviceName());
    if (device != NULL && device->isConnected())
    {
        device->clearChannel(this->command.getChannel());
        device->clearMixerChannel(this->command.getChannel());
    }

    foreach (const DeviceModel& model, DeviceManager::getInstance().getDeviceModels())
    {
        if (model.getShadow() == "No")
            continue;

        const QSharedPointer<CasparDevice> deviceShadow = DeviceManager::getInstance().getConnectionByName(model.getName());
        if (deviceShadow != NULL && deviceShadow->isConnected())
        {
            deviceShadow->clearChannel(this->command.getChannel());
            deviceShadow->clearMixerChannel(this->command.getChannel());
        }
    }
}

void RundownGeometryWidget::channelChanged(int channel)
{
    this->labelChannel->setText(QString("Channel: %1").arg(channel));
}

void RundownGeometryWidget::videolayerChanged(int videolayer)
{
    this->labelVideolayer->setText(QString("Videolayer: %1").arg(videolayer));
}

void RundownGeometryWidget::delayChanged(int delay)
{
    this->labelDelay->setText(QString("Delay: %1").arg(delay));
}

void RundownGeometryWidget::checkGpiTriggerable()
{
    labelGpiTriggerable->setVisible(this->command.getAllowGpi());

    if (GpiManager::getInstance().getGpiDevice()->isConnected())
        labelGpiTriggerable->setPixmap(QPixmap(":/Graphics/Images/GpiTriggerable.png"));
    else
        labelGpiTriggerable->setPixmap(QPixmap(":/Graphics/Images/GpiTriggerableDisconnected.png"));
}

void RundownGeometryWidget::allowGpiChanged(bool allowGpi)
{
    checkGpiTriggerable();
}

void RundownGeometryWidget::gpiDeviceConnected(bool connected, GpiDevice* device)
{
    checkGpiTriggerable();
}
