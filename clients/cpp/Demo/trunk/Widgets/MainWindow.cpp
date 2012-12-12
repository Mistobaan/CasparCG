#include "MainWindow.h"

#include "Enum.h"
#include "Global.h"

#include "DeviceManager.h"

#include "AboutDialog.h"
#include "BigFourWidget.h"
#include "FrameWidget.h"
#include "RecorderWidget.h"
#include "StatusbarEvent.h"
#include "SqueezeWidget.h"
#include "StartWidget.h"

#include <QtGui/QDesktopWidget>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMouseEvent>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setupUi(this);
    setupUiMenu();
    setWindowIcon(QIcon(":/Graphics/Images/CasparCG.ico"));
    setWindowTitle(QString("%1 %2.%3").arg(this->windowTitle()).arg(MAJOR_VERSION).arg(MINOR_VERSION));

    this->stackedLayout = new QStackedLayout();
    stackedLayout->addWidget(new StartWidget(this));
    stackedLayout->addWidget(new RecorderWidget(this));
    stackedLayout->addWidget(new FrameWidget(this));
    stackedLayout->addWidget(new BigFourWidget(this));
    stackedLayout->addWidget(new SqueezeWidget(this));
    this->frameWidgets->setLayout(stackedLayout);

    qApp->installEventFilter(this);

    QObject::connect(&DeviceManager::getInstance().getDevice(), SIGNAL(connectionStateChanged(CasparDevice&)), this, SLOT(deviceConnectionStateChanged(CasparDevice&)));
}

void MainWindow::setupUiMenu()
{
     QMenu* fileMenu = new QMenu(this);
     fileMenu->addAction("&Quit", this, SLOT(close()));

     QMenu* helpMenu = new QMenu(this);
     helpMenu->addAction("&About CasparCG Demo", this, SLOT(showAboutDialog()));

     QMenuBar* menuBar = new QMenuBar(this);
     menuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
     menuBar->addMenu(fileMenu)->setText("&File");
     menuBar->addMenu(helpMenu)->setText("&Help");

     this->setMenuBar(menuBar);
}

bool MainWindow::eventFilter(QObject* target, QEvent* event)
{
    if(event->type() == static_cast<QEvent::Type>(Enum::EventType::StatusbarMessage))
    {
        StatusbarEvent* statusbarEvent = static_cast<StatusbarEvent*>(event);
        if (statusbarEvent->getTimeout() == 0)
            statusBar()->showMessage(statusbarEvent->getMessage());
        else
            statusBar()->showMessage(statusbarEvent->getMessage(), statusbarEvent->getTimeout());
    }

    return QObject::eventFilter(target, event);
}

void MainWindow::deviceConnectionStateChanged(CasparDevice& device)
{
    if (device.isConnected())
        showStart();
    else
        foreach(QPushButton* button, this->frameButtons->findChildren<QPushButton*>())
            button->setEnabled(false);
}

void MainWindow::enableDemoButton(const QString& buttonName)
{
    foreach(QPushButton* button, this->frameButtons->findChildren<QPushButton*>())
        button->setEnabled(true);

    this->frameButtons->findChild<QPushButton*>(buttonName)->setEnabled(false);
}

void MainWindow::showAboutDialog()
{
    AboutDialog* dialog = new AboutDialog(this);
    dialog->show();
}

void MainWindow::showStart()
{
    enableDemoButton("pushButtonStart");
    this->stackedLayout->setCurrentIndex(0);
}

void MainWindow::showRecorder()
{
    enableDemoButton("pushButtonRecorder");
    this->stackedLayout->setCurrentIndex(1);
}

void MainWindow::showFrame()
{
    enableDemoButton("pushButtonFrame");
    this->stackedLayout->setCurrentIndex(2);
}

void MainWindow::showBigFour()
{
    enableDemoButton("pushButtonBigFour");
    this->stackedLayout->setCurrentIndex(3);
}

void MainWindow::showSqueeze()
{
    enableDemoButton("pushButtonSqueeze");
    this->stackedLayout->setCurrentIndex(4);
}
