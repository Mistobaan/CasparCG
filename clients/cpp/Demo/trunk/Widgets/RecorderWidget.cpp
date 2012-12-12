#include "RecorderWidget.h"

#include "DeviceManager.h"

RecorderWidget::RecorderWidget(QWidget* parent) : QWidget(parent)
{
    setupUi(this);

    this->isRecording = false;

    this->lcdNumber->hide();
    this->lcdNumber->display("00:00:00:00");

    this->timer = new QTimer(this);
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
}

bool RecorderWidget::eventFilter(QObject* target, QEvent* event)
{
    //if(event->type() == static_cast<QEvent::Type>(EventEnum::Statusbar))
    //{
    //    StatusbarEvent* statusbarEvent = static_cast<StatusbarEvent*>(event);
    //    statusBar()->showMessage(statusbarEvent->getMessage(), 5000);
    //}

    return QObject::eventFilter(target, event);
}

void RecorderWidget::updateTimer()
{
    QString text;

    int ms = this->time.elapsed();
    int s  = ms / 1000;
    int m  = s  / 60;
    int h  = m  / 60;

    if(h < 10)
        text.append("0");

    text.append(QString("%1:").arg(h));

    if(m < 10)
        text.append("0");

    text.append(QString("%1:").arg(m));

    if(s < 10)
        text.append("0");

    text.append(QString("%1:").arg(s));

    if(ms < 10)
        text.append("0");

    QString milliseconds = QString::number(ms);
    text.append(milliseconds.leftJustified(2, ' ', true));

    this->lcdNumber->display(text);

    if(this->lcdNumber->isHidden())
        this->lcdNumber->show();
}

void RecorderWidget::buttonPressed()
{
    if (!this->isRecording)
    {
        QString codec = this->comboBoxCodec->currentText().toLower();
        if (this->comboBoxCodec->currentIndex() == 0)
            codec = "libx264";

        if (this->lineEditFilename->text().isEmpty())
            DeviceManager::getInstance().getDevice().startRecording(1, this->lineEditFilename->placeholderText(), QString("-vcodec %1").arg(codec));
        else
            DeviceManager::getInstance().getDevice().startRecording(1, this->lineEditFilename->text(), QString("-vcodec %1").arg(codec));

        this->pushButton->setIcon(QIcon(":/Graphics/Images/Recording.png"));

        this->time.start();
        this->timer->start();
        this->isRecording = true;
    }
    else
    {
        DeviceManager::getInstance().getDevice().stopRecording(1);

        this->pushButton->setIcon(QIcon(":/Graphics/Images/Record.png"));

        this->isRecording = false;

        this->timer->stop();
    }
}
