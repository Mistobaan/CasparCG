#include "BrightnessCommand.h"

#include "Global.h"

BrightnessCommand::BrightnessCommand(QObject* parent)
    : QObject(parent),
      channel(Output::DEFAULT_CHANNEL), videolayer(Output::DEFAULT_VIDEOLAYER), delay(Output::DEFAULT_DELAY),
      allowGpi(Output::DEFAULT_ALLOW_GPI), brightness(Mixer::DEFAULT_BRIGHTNESS), duration(Mixer::DEFAULT_DURATION),
      tween(Mixer::DEFAULT_TWEEN), defer(Mixer::DEFAULT_DEFER)
{
}

int BrightnessCommand::getDelay() const
{
    return this->delay;
}

int BrightnessCommand::getChannel() const
{
    return this->channel;
}

int BrightnessCommand::getVideolayer() const
{
    return this->videolayer;
}

void BrightnessCommand::setChannel(int channel)
{
    this->channel = channel;
    emit channelChanged(this->channel);
}

void BrightnessCommand::setVideolayer(int videolayer)
{
    this->videolayer = videolayer;
    emit videolayerChanged(this->videolayer);
}

void BrightnessCommand::setDelay(int delay)
{
    this->delay = delay;
    emit delayChanged(this->delay);
}

float BrightnessCommand::getBrightness() const
{
    return this->brightness;
}

int BrightnessCommand::getDuration() const
{
    return this->duration;
}

const QString& BrightnessCommand::getTween() const
{
    return this->tween;
}

bool BrightnessCommand::getDefer() const
{
    return this->defer;
}

void BrightnessCommand::setBrightness(float brightness)
{
    this->brightness = brightness;
    emit brightnessChanged(this->brightness);
}

void BrightnessCommand::setDuration(int duration)
{
    this->duration = duration;
    emit durationChanged(this->duration);
}

void BrightnessCommand::setTween(const QString& tween)
{
    this->tween = tween;
    emit tweenChanged(this->tween);
}

void BrightnessCommand::setDefer(bool defer)
{
    this->defer = defer;
    emit deferChanged(this->defer);
}

bool BrightnessCommand::getAllowGpi() const
{
    return this->allowGpi;
}

void BrightnessCommand::setAllowGpi(bool allowGpi)
{
    this->allowGpi = allowGpi;
    emit allowGpiChanged(this->allowGpi);
}

void BrightnessCommand::loadProperties(const boost::property_tree::wptree& pt)
{
    setChannel(pt.get<int>(L"channel"));
    setVideolayer(pt.get<int>(L"videolayer"));
    setDelay(pt.get<int>(L"delay"));
    setAllowGpi(pt.get<bool>(L"allowgpi"));
    setBrightness(pt.get<float>(L"brightness"));
    setDuration(pt.get<int>(L"duration"));
    setTween(QString::fromStdWString(pt.get<std::wstring>(L"tween")));
    setDefer(pt.get<bool>(L"defer"));
}

void BrightnessCommand::saveProperties(const boost::property_tree::wptree& pt)
{
}
