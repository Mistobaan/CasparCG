#include "CommitCommand.h"

#include "Global.h"

CommitCommand::CommitCommand(QObject* parent)
    : QObject(parent),
      channel(Output::DEFAULT_CHANNEL), videolayer(Output::DEFAULT_VIDEOLAYER), delay(Output::DEFAULT_DELAY),
      allowGpi(Output::DEFAULT_ALLOW_GPI)
{
}

int CommitCommand::getDelay() const
{
    return this->delay;
}

bool CommitCommand::getAllowGpi() const
{
    return this->allowGpi;
}

int CommitCommand::getChannel() const
{
    return this->channel;
}

int CommitCommand::getVideolayer() const
{
    return this->videolayer;
}

void CommitCommand::setChannel(int channel)
{
    this->channel = channel;
    emit channelChanged(this->channel);
}

void CommitCommand::setVideolayer(int videolayer)
{
    this->videolayer = videolayer;
    emit videolayerChanged(this->videolayer);
}

void CommitCommand::setDelay(int delay)
{
    this->delay = delay;
    emit delayChanged(this->delay);
}

void CommitCommand::setAllowGpi(bool allowGpi)
{
    this->allowGpi = allowGpi;
    emit allowGpiChanged(this->allowGpi);
}

void CommitCommand::loadProperties(const boost::property_tree::wptree& pt)
{
    setChannel(pt.get<int>(L"channel"));
    setDelay(pt.get<int>(L"delay"));
    setAllowGpi(pt.get<bool>(L"allowgpi"));
}

void CommitCommand::saveProperties(const boost::property_tree::wptree& pt)
{
}
