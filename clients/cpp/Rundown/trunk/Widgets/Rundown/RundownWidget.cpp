#include "RundownWidget.h"
#include "RundownBlendWidget.h"
#include "RundownBrightnessWidget.h"
#include "RundownCommitWidget.h"
#include "RundownContrastWidget.h"
#include "RundownCropWidget.h"
#include "RundownDeckLinkInputWidget.h"
#include "RundownFileRecorderWidget.h"
#include "RundownImageScrollerWidget.h"
#include "RundownGeometryWidget.h"
#include "RundownGpiOutputWidget.h"
#include "RundownGridWidget.h"
#include "RundownGroupWidget.h"
#include "RundownKeyerWidget.h"
#include "RundownLevelsWidget.h"
#include "RundownMediaWidget.h"
#include "RundownOpacityWidget.h"
#include "RundownSaturationWidget.h"
#include "RundownTemplateWidget.h"
#include "RundownVolumeWidget.h"

#include "DatabaseManager.h"
#include "Events/AddRudnownItemEvent.h"
#include "Events/OpenRundownEvent.h"
#include "Events/RundownIsEmptyEvent.h"
#include "Events/RundownItemSelectedEvent.h"
#include "Events/StatusbarEvent.h"
#include "GpiManager.h"
#include "Models/RundownModel.h"

#include <iostream>

#include <QtCore/QDebug>
#include <QtCore/QPoint>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QTreeWidgetItem>

RundownWidget::RundownWidget(QWidget* parent) : QWidget(parent), isEnterPressed(false)
{
    setupUi(this);
    setupUiMenu();

    this->lineEditFilter->setVisible(false);

    // TODO: specific Gpi device.
    QObject::connect(GpiManager::getInstance().getGpiDevice().data(), SIGNAL(gpiTriggered(int, GpiDevice*)), this, SLOT(gpiPortTriggered(int, GpiDevice*)));
    QObject::connect(this->treeWidgetRundown, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(customContextMenuRequested(const QPoint &)));

    foreach (const GpiPortModel& port, DatabaseManager::getInstance().getGpiPorts())
        gpiBindingChanged(port.getPort(), port.getAction());

    checkEmptyRundown();

    qApp->installEventFilter(this);
}

void RundownWidget::setupUiMenu()
{
    this->mixerMenu = new QMenu(this);
    this->mixerMenu->setTitle("Mixer");
    //this->mixerMenu->setIcon(QIcon(":/Graphics/Images/Mixer.png"));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Blending", this, SLOT(addBlendModeCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Brightness", this, SLOT(addBrightnessCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Contrast", this, SLOT(addContrastCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Crop", this, SLOT(addCropCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Geometry", this, SLOT(addGeometryCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Grid", this, SLOT(addGridCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Keyer", this, SLOT(addKeyerCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Levels", this, SLOT(addLevelsCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Opacity", this, SLOT(addOpacityCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Saturation", this, SLOT(addSaturationCommand()));
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Volume", this, SLOT(addVolumeCommand()));
    this->mixerMenu->addSeparator();
    this->mixerMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Commit", this, SLOT(addCommitCommand()));

    this->libraryMenu = new QMenu(this);
    this->libraryMenu->setTitle("Library");
    //this->mixerMenu->setIcon(QIcon(":/Graphics/Images/Library.png"));
    this->libraryMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Audio", this, SLOT(addAudioCommand()));
    this->libraryMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Image", this, SLOT(addImageCommand()));
    this->libraryMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Template", this, SLOT(addTemplateCommand()));
    this->libraryMenu->addAction(/*QIcon(":/Graphics/Images/Mixer.png"),*/ "Video", this, SLOT(addVideoCommand()));

    this->newMenu = new QMenu(this);
    this->newMenu->setTitle("New");
    //this->newMenu->setIcon(QIcon(":/Graphics/Images/New.png"));
    this->newMenu->addMenu(this->mixerMenu);
    this->newMenu->addMenu(this->libraryMenu);
    this->newMenu->addSeparator();
    this->newMenu->addAction(/*QIcon(":/Graphics/Images/Gpi.png"),*/ "GPI Output", this, SLOT(addGpiOutputCommand()));
    this->newMenu->addAction(/*QIcon(":/Graphics/Images/Producer.png"),*/ "Color Producer", this, SLOT(addFileRecorderCommand()));
    this->newMenu->addAction(/*QIcon(":/Graphics/Images/Consumer.png"),*/ "File Recorder", this, SLOT(addFileRecorderCommand()));
    this->newMenu->addAction(/*QIcon(":/Graphics/Images/Consumer.png"),*/ "Image Scroller", this, SLOT(addImageScrollerCommand()));
    this->newMenu->addAction(/*QIcon(":/Graphics/Images/Producer.png"),*/ "DeckLink Input", this, SLOT(addDeckLinkInputCommand()));
    this->newMenu->actions().at(4)->setEnabled(false);

    this->colorMenu = new QMenu(this);
    this->colorMenu->setTitle("Colorize");
    //this->colorMenu->setIcon(QIcon(":/Graphics/Images/Color.png"));
    this->colorMenu->addAction(/*QIcon(":/Graphics/Images/Color.png"),*/ "Sienna");
    this->colorMenu->addAction(/*QIcon(":/Graphics/Images/Color.png"),*/ "Goldenrod");
    this->colorMenu->addAction(/*QIcon(":/Graphics/Images/Color.png"),*/ "OliveDrab");
    this->colorMenu->addAction(/*QIcon(":/Graphics/Images/Color.png"),*/ "SeaGreen");
    this->colorMenu->addAction(/*QIcon(":/Graphics/Images/Color.png"),*/ "Chocolate");
    this->colorMenu->addAction(/*QIcon(":/Graphics/Images/Color.png"),*/ "DarkSlateGray");
    this->colorMenu->addAction(/*QIcon(":/Graphics/Images/Color.png"),*/ "CadetBlue");

    this->contextMenu = new QMenu(this);
    this->contextMenu->addMenu(this->newMenu);
    this->contextMenu->addSeparator();
    this->contextMenu->addAction(/*QIcon(":/Graphics/Images/Group.png"),*/ "Group");
    this->contextMenu->addAction(/*QIcon(":/Graphics/Images/Ungroup.png"),*/ "Ungroup");
    this->contextMenu->addSeparator();
    this->contextMenu->addMenu(this->colorMenu);

    QObject::connect(this->newMenu, SIGNAL(triggered(QAction*)), this, SLOT(newMenuTriggered(QAction*)));
    QObject::connect(this->colorMenu, SIGNAL(triggered(QAction*)), this, SLOT(colorMenuTriggered(QAction*)));
    QObject::connect(this->contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuTriggered(QAction*)));
}

void RundownWidget::parseGroup(const QString& name, const QString& deviceName, const QString& type, const boost::property_tree::wptree& pt)
{
    IRundownWidget* widget = new RundownGroupWidget(LibraryModel(-1, name, deviceName, type), this);
    widget->setActive(true);
    widget->setExpanded(true);

    widget->getCommand()->loadProperties(pt);

    QTreeWidgetItem* parent = new QTreeWidgetItem();
    this->treeWidgetRundown->invisibleRootItem()->addChild(parent);
    this->treeWidgetRundown->setItemWidget(parent, 0, dynamic_cast<QWidget*>(widget));
    this->treeWidgetRundown->expandItem(parent);
    this->treeWidgetRundown->doItemsLayout(); // Refresh.

    BOOST_FOREACH(const boost::property_tree::wptree::value_type& value, pt.get_child(L"items"))
    {
        QString type = QString::fromStdWString(value.second.get<std::wstring>(L"type")).toUpper();
        QString deviceName = QString::fromStdWString(value.second.get<std::wstring>(L"devicename"));
        QString name = QString::fromStdWString(value.second.get<std::wstring>(L"name"));

        parseItem(name, deviceName, type, value.second, parent);
    }
}

void RundownWidget::parseItem(const QString& name, const QString& deviceName, const QString& type, const boost::property_tree::wptree& pt, QTreeWidgetItem* parent)
{
    IRundownWidget* widget = NULL;
    if (type == "BLENDMODE")
        widget = new RundownBlendWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "BRIGHTNESS")
        widget = new RundownBrightnessWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "CONTRAST")
        widget = new RundownContrastWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "CROP")
        widget = new RundownCropWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "DECKLINKINPUT")
        widget = new RundownDeckLinkInputWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type ==  "FILERECORDER")
        widget = new RundownFileRecorderWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "GEOMETRY")
        widget = new RundownGeometryWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "GRID")
        widget = new RundownGridWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "GPIOUTPUT")
        widget = new RundownGpiOutputWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "KEYER")
        widget = new RundownKeyerWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "LEVELS")
        widget = new RundownLevelsWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "AUDIO" || type == "STILL" || type == "MOVIE")
        widget = new RundownMediaWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "OPACITY")
        widget = new RundownOpacityWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "SATURATION")
        widget = new RundownSaturationWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "TEMPLATE")
        widget = new RundownTemplateWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "VOLUME")
        widget = new RundownVolumeWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "COMMIT")
        widget = new RundownCommitWidget(LibraryModel(-1, name, deviceName, type), this);
    else if (type == "IMAGESCROLLER")
        widget = new RundownImageScrollerWidget(LibraryModel(-1, name, deviceName, type), this);

    widget->getCommand()->loadProperties(pt);

    QTreeWidgetItem* child = new QTreeWidgetItem();
    if (parent == NULL) // Top level item.
        this->treeWidgetRundown->invisibleRootItem()->addChild(child);
    else
        parent->addChild(child);

    this->treeWidgetRundown->setItemWidget(child, 0, dynamic_cast<QWidget*>(widget));
    this->treeWidgetRundown->doItemsLayout(); // Refresh.
}

bool RundownWidget::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_F1) // Stop.
            return executeCommand(Playout::PlayoutType::Stop, KeyPress);
        else if (keyEvent->key() == Qt::Key_F2) // Play.
            return executeCommand(Playout::PlayoutType::Play, KeyPress);
        else if (keyEvent->key() == Qt::Key_F3) // Pause.
            return executeCommand(Playout::PlayoutType::Pause, KeyPress);
        else if (keyEvent->key() == Qt::Key_F5) // Load.
            return executeCommand(Playout::PlayoutType::Load, KeyPress);
        else if (keyEvent->key() == Qt::Key_F6) // Next.
            return executeCommand(Playout::PlayoutType::Next, KeyPress);
        else if (keyEvent->key() == Qt::Key_F7) // Update.
            return executeCommand(Playout::PlayoutType::Update, KeyPress);
        else if (keyEvent->key() == Qt::Key_F8) // Invoke.
            return executeCommand(Playout::PlayoutType::Invoke, KeyPress);
        else if (keyEvent->key() == Qt::Key_F9) // Clear.
            return executeCommand(Playout::PlayoutType::Clear, KeyPress);
        else if (keyEvent->key() == Qt::Key_F10) // Clear videolayer.
            return executeCommand(Playout::PlayoutType::ClearVideolayer, KeyPress);
        else if (keyEvent->key() == Qt::Key_F11) // Clear channel.
            return executeCommand(Playout::PlayoutType::ClearChannel, KeyPress);

        if (target == treeWidgetRundown)
        {  
            if (keyEvent->key() == Qt::Key_Delete)
                return removeSelectedItems();
            else if (keyEvent->key() == Qt::Key_C && keyEvent->modifiers() == Qt::ControlModifier)
                return copySelectedItem();
            else if (keyEvent->key() == Qt::Key_V && keyEvent->modifiers() == Qt::ControlModifier)
                return pasteSelectedItem();
            else if (keyEvent->key() == Qt::Key_G && keyEvent->modifiers() == Qt::ControlModifier)
                return groupItems();
            else if (keyEvent->key() == Qt::Key_U && keyEvent->modifiers() == Qt::ControlModifier)
                return ungroupItems();
            else if (keyEvent->key() == Qt::Key_Up && keyEvent->modifiers() == Qt::ControlModifier)
                return moveItemUp();
            else if (keyEvent->key() == Qt::Key_Down && keyEvent->modifiers() == Qt::ControlModifier)
                return moveItemDown();
            else if (keyEvent->key() == Qt::Key_Left && keyEvent->modifiers() == Qt::ControlModifier)
                return moveItemOutOfGroup();
            else if (keyEvent->key() == Qt::Key_Right && keyEvent->modifiers() == Qt::ControlModifier)
                return moveItemIntoGroup();
        }

        /*
        if (target == treeWidgetRundown)
        {
            QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
            parsePage(keyEvent);

            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Plus || keyEvent->key() == Qt::Key_Minus)
                playSelectedEvent();
        }
        */
    }
    else if (event->type() == static_cast<QEvent::Type>(Enum::EventType::OpenRundown))
    {
        OpenRundownEvent* openRundownEvent = dynamic_cast<OpenRundownEvent*>(event);
        QFile file(openRundownEvent->getPath());
        if (file.open(QFile::ReadOnly))
        {
            QTextStream stream(&file);
            std::wstringstream wstringstream;

            wstringstream << stream.readAll().toStdWString();

            file.close();

            boost::property_tree::wptree pt;
            boost::property_tree::xml_parser::read_xml(wstringstream, pt);

            this->treeWidgetRundown->selectAll();
            removeSelectedItems();

            BOOST_FOREACH(const boost::property_tree::wptree::value_type& value, pt.get_child(L"items"))
            {
                QString type = QString::fromStdWString(value.second.get<std::wstring>(L"type")).toUpper();
                QString deviceName = QString::fromStdWString(value.second.get<std::wstring>(L"devicename"));
                QString name = QString::fromStdWString(value.second.get<std::wstring>(L"name"));

                if (type == "GROUP")
                    parseGroup(name, deviceName, type, value.second);
                else
                    parseItem(name, deviceName, type, value.second, NULL);
            }

            if (this->treeWidgetRundown->invisibleRootItem()->childCount() > 0)
                this->treeWidgetRundown->setCurrentItem(this->treeWidgetRundown->invisibleRootItem()->child(0));

            this->treeWidgetRundown->setFocus();
        }

        checkEmptyRundown();

        return true;
    }
    else if (event->type() == static_cast<QEvent::Type>(Enum::EventType::AddRudnownItem))
    {
        AddRudnownItemEvent* addRudnownItemEvent = dynamic_cast<AddRudnownItemEvent*>(event);

        IRundownWidget* widget = NULL;
        if (addRudnownItemEvent->getLibraryModel().getType() == "BLENDMODE")
            widget = new RundownBlendWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "BRIGHTNESS")
            widget = new RundownBrightnessWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "CONTRAST")
            widget = new RundownContrastWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "CROP")
            widget = new RundownCropWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "DECKLINKINPUT")
            widget = new RundownDeckLinkInputWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "FILERECORDER")
            widget = new RundownFileRecorderWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "GEOMETRY")
            widget = new RundownGeometryWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "GRID")
            widget = new RundownGridWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "GPIOUTPUT")
            widget = new RundownGpiOutputWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "KEYER")
            widget = new RundownKeyerWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "LEVELS")
            widget = new RundownLevelsWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "AUDIO" ||
                 addRudnownItemEvent->getLibraryModel().getType() == "STILL" ||
                 addRudnownItemEvent->getLibraryModel().getType() == "MOVIE")
            widget = new RundownMediaWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "OPACITY")
            widget = new RundownOpacityWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "SATURATION")
            widget = new RundownSaturationWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "TEMPLATE")
            widget = new RundownTemplateWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "VOLUME")
            widget = new RundownVolumeWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "COMMIT")
            widget = new RundownCommitWidget(addRudnownItemEvent->getLibraryModel(), this);
        else if (addRudnownItemEvent->getLibraryModel().getType() == "IMAGESCROLLER")
            widget = new RundownImageScrollerWidget(addRudnownItemEvent->getLibraryModel(), this);

        QTreeWidgetItem* item = new QTreeWidgetItem();
        if (this->treeWidgetRundown->currentItem() == NULL) // There is no item selected.
            this->treeWidgetRundown->invisibleRootItem()->addChild(item); // Add item to the bottom of the rundown.
        else if (this->treeWidgetRundown->currentItem()->parent() == NULL) // Top level item.
            this->treeWidgetRundown->invisibleRootItem()->insertChild(this->treeWidgetRundown->currentIndex().row() + 1, item); // Insert item below.
        else if (this->treeWidgetRundown->currentItem()->parent() != NULL) // Goup item.
        {
            this->treeWidgetRundown->currentItem()->parent()->insertChild(this->treeWidgetRundown->currentIndex().row() + 1, item); // Insert item below.
            widget->setInGroup(true);
        }

        this->treeWidgetRundown->setItemWidget(item, 0, dynamic_cast<QWidget*>(widget));
        this->treeWidgetRundown->setCurrentItem(item);
        this->treeWidgetRundown->setFocus();
        this->treeWidgetRundown->doItemsLayout(); // Refresh.

        checkEmptyRundown();

        return true;
    }

    return QObject::eventFilter(target, event);
}

void RundownWidget::checkEmptyRundown()
{
    this->treeWidgetRundown->setStyleSheet((this->treeWidgetRundown->invisibleRootItem()->childCount() == 0) ? "border-color: red;" : "");
}

void RundownWidget::colorizeItems(const QString& color)
{
    if (this->treeWidgetRundown->selectedItems().count() == 0)
        return;

    foreach (QTreeWidgetItem* item, this->treeWidgetRundown->selectedItems())
        dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(item, 0))->setColor(color); // Colorize current selected item.
}

void RundownWidget::gpiPortTriggered(int gpiPort, GpiDevice* device)
{
    executeCommand(gpiBindings[gpiPort], GpiPulse);
}

void RundownWidget::gpiBindingChanged(int gpiPort, Playout::PlayoutType::Type binding)
{
    gpiBindings[gpiPort] = binding;
}

void RundownWidget::customContextMenuRequested(const QPoint& point)
{
    foreach (QAction* action, this->contextMenu->actions())
        action->setEnabled(true);

    bool isGroup = false;
    bool isTopItem = false;
    bool isGroupItem = false;
    foreach (QTreeWidgetItem* item, this->treeWidgetRundown->selectedItems())
    {
        QWidget* widget = this->treeWidgetRundown->itemWidget(item, 0);

        if (item->parent() != NULL) // Group item.
            isGroupItem = true;
        else if (dynamic_cast<IRundownWidget*>(widget)->isGroup()) // Group
            isGroup = true;
        else if (item->parent() == NULL && !dynamic_cast<IRundownWidget*>(widget)->isGroup()) // Top level item.
            isTopItem = true;
    }

    if (isGroup)
    {
        this->contextMenu->actions().at(2)->setEnabled(false); // We don't support group in groups.
        this->contextMenu->actions().at(5)->setEnabled(false); // We don't support colorize groups.
    }

    if (isGroupItem)
        this->contextMenu->actions().at(2)->setEnabled(false); // We don't support group in groups.

    if (isTopItem || (isGroup && isGroupItem) || (isTopItem && isGroupItem))
        this->contextMenu->actions().at(3)->setEnabled(false); // We don't have any group to ungroup.

    if (!isTopItem && !isGroup && !isGroupItem)
    {
        this->contextMenu->actions().at(2)->setEnabled(false); // Group.
        this->contextMenu->actions().at(3)->setEnabled(false); // Ungroup.
        this->contextMenu->actions().at(5)->setEnabled(false); // Colorize.
    }

    this->contextMenu->exec(this->treeWidgetRundown->mapToGlobal(point));
}

void RundownWidget::newMenuTriggered(QAction* action)
{
}

void RundownWidget::colorMenuTriggered(QAction* action)
{
    colorizeItems(action->text());
}

void RundownWidget::contextMenuTriggered(QAction* action)
{
    if (action->text() == "Group")
        groupItems();
    else if (action->text() == "Ungroup")
        ungroupItems();
}

void RundownWidget::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    QWidget* currentWidget = this->treeWidgetRundown->itemWidget(current, 0);
    QWidget* previousWidget = this->treeWidgetRundown->itemWidget(previous, 0);

    if (previous != NULL && previousWidget != NULL)
        dynamic_cast<IRundownWidget*>(previousWidget)->setActive(false);

    if (current != NULL && currentWidget != NULL)
        dynamic_cast<IRundownWidget*>(currentWidget)->setActive(true);

    QTreeWidgetItem* currentItem = this->treeWidgetRundown->currentItem();
    QWidget* currentItemWidget = this->treeWidgetRundown->itemWidget(this->treeWidgetRundown->currentItem(), 0);
    if (currentItem != NULL && currentItemWidget != NULL)
    {
        ICommand* command = dynamic_cast<IRundownWidget*>(currentItemWidget)->getCommand();
        LibraryModel* model = dynamic_cast<IRundownWidget*>(currentItemWidget)->getLibraryModel();

        qApp->postEvent(qApp, new RundownItemSelectedEvent(command, model));
    }
    else if (currentItem == NULL && previous != NULL && this->treeWidgetRundown->invisibleRootItem()->childCount() == 1) // Last item was removed form the rundown.
    {
        qApp->postEvent(qApp, new RundownIsEmptyEvent());
    }
}

void RundownWidget::itemDoubleClicked(QTreeWidgetItem* item, int index)
{
    QWidget* selectedWidget = this->treeWidgetRundown->itemWidget(this->treeWidgetRundown->currentItem(), 0);
    IRundownWidget* rundownWidget = dynamic_cast<IRundownWidget*>(selectedWidget);

    if (rundownWidget->isGroup()) // Group.
        rundownWidget->setExpanded(!item->isExpanded());
}

bool RundownWidget::copySelectedItem()
{
    if (this->treeWidgetRundown->currentItem() == NULL)
        return false;

    this->copyItem = this->treeWidgetRundown->currentItem();

    return true;
}

bool RundownWidget::pasteSelectedItem()
{
    if (this->treeWidgetRundown->currentItem() == NULL || this->copyItem == NULL)
        return false;

    if (this->treeWidgetRundown->currentItem()->parent() != NULL && dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(this->copyItem, 0))->isGroup())
        return false; // We don't support group in groups.

    int row  = this->treeWidgetRundown->currentIndex().row();

    QTreeWidgetItem* parentItem = new QTreeWidgetItem();
    IRundownWidget* parentWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(this->copyItem, 0))->clone();
    parentWidget->setActive(false);

    if (this->treeWidgetRundown->currentItem()->parent() == NULL) // Top level item.
    {
        parentWidget->setInGroup(false);
        parentWidget->setExpanded(false);

        this->treeWidgetRundown->invisibleRootItem()->insertChild(row + 1, parentItem);
    }
    else
    {
        parentWidget->setInGroup(true);
        parentWidget->setExpanded(true);

        this->treeWidgetRundown->currentItem()->parent()->insertChild(row + 1, parentItem);
    }

    this->treeWidgetRundown->setItemWidget(parentItem, 0, dynamic_cast<QWidget*>(parentWidget));

    for (int i = 0; i < this->copyItem->childCount(); i++)
    {
        QTreeWidgetItem* item = this->copyItem->child(i);

        IRundownWidget* childWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(item, 0))->clone();

        QTreeWidgetItem* childItem = new QTreeWidgetItem();
        parentItem->addChild(childItem);
        this->treeWidgetRundown->setItemWidget(childItem, 0, dynamic_cast<QWidget*>(childWidget));
    }

    if (this->copyItem->isExpanded())
        this->treeWidgetRundown->expandItem(parentItem);

    this->treeWidgetRundown->doItemsLayout(); // Refresh

    return true;
}

bool RundownWidget::moveItemDown()
{
    if (this->treeWidgetRundown->currentItem() == NULL)
        return false;

    int row  = this->treeWidgetRundown->currentIndex().row();
    QTreeWidgetItem* currentItem = this->treeWidgetRundown->currentItem();
    QTreeWidgetItem* parentItem = this->treeWidgetRundown->currentItem()->parent();

    if (dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->isGroup())
    {
        int rowCount = 0;
        if (parentItem == NULL) // Top level item.
            rowCount = this->treeWidgetRundown->invisibleRootItem()->childCount() - 1;

        if (currentItem != NULL && row < rowCount)
        {
            IRundownWidget* parentWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->clone();
            parentWidget->setInGroup(true);
            parentWidget->setExpanded(true);

            QTreeWidgetItem* parentItem = new QTreeWidgetItem();
            this->treeWidgetRundown->invisibleRootItem()->insertChild(row + 2, parentItem);
            this->treeWidgetRundown->setItemWidget(parentItem, 0, dynamic_cast<QWidget*>(parentWidget));

            if (this->treeWidgetRundown->currentItem()->isExpanded())
                this->treeWidgetRundown->expandItem(parentItem);

            for (int i = 0; i < this->treeWidgetRundown->currentItem()->childCount(); i++)
            {
                QTreeWidgetItem* item = this->treeWidgetRundown->currentItem()->child(i);

                IRundownWidget* childWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(item, 0))->clone();

                QTreeWidgetItem* childItem = new QTreeWidgetItem();
                parentItem->addChild(childItem);
                this->treeWidgetRundown->setItemWidget(childItem, 0, dynamic_cast<QWidget*>(childWidget));
            }

            delete currentItem;

            this->treeWidgetRundown->setCurrentItem(parentItem);
            this->treeWidgetRundown->doItemsLayout(); // Refresh
        }
    }
    else
    {
        int rowCount = 0;
        if (parentItem == NULL) // Top level item.
            rowCount = this->treeWidgetRundown->invisibleRootItem()->childCount() - 1;
        else
            rowCount = this->treeWidgetRundown->currentItem()->parent()->childCount() - 1;

        if (currentItem != NULL && row < rowCount)
        {
            IRundownWidget* newWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->clone();

            if (parentItem == NULL) // Top level item.
            {
                this->treeWidgetRundown->invisibleRootItem()->takeChild(row);
                this->treeWidgetRundown->invisibleRootItem()->insertChild(row + 1, currentItem);
            }
            else // Group item.
            {
                this->treeWidgetRundown->currentItem()->parent()->takeChild(row);
                this->treeWidgetRundown->currentItem()->parent()->insertChild(row + 1, currentItem);
            }

            this->treeWidgetRundown->setItemWidget(currentItem, 0, dynamic_cast<QWidget*>(newWidget));
            this->treeWidgetRundown->setCurrentItem(currentItem);
            this->treeWidgetRundown->doItemsLayout(); // Refresh
        }
    }

    return true;
}

bool RundownWidget::moveItemUp()
{
    if (this->treeWidgetRundown->currentItem() == NULL)
        return false;

    int row  = this->treeWidgetRundown->currentIndex().row();
    QTreeWidgetItem* currentItem = this->treeWidgetRundown->currentItem();
    QTreeWidgetItem* parentItem = this->treeWidgetRundown->currentItem()->parent();

    if (dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->isGroup())
    {
        int rowCount = 0;
        if (currentItem != NULL && row > rowCount)
        {
            IRundownWidget* parentWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->clone();
            parentWidget->setInGroup(true);
            parentWidget->setExpanded(true);

            QTreeWidgetItem* parentItem = new QTreeWidgetItem();
            this->treeWidgetRundown->invisibleRootItem()->insertChild(row - 1, parentItem);
            this->treeWidgetRundown->setItemWidget(parentItem, 0, dynamic_cast<QWidget*>(parentWidget));

            if (this->treeWidgetRundown->currentItem()->isExpanded())
                this->treeWidgetRundown->expandItem(parentItem);

            for (int i = 0; i < this->treeWidgetRundown->currentItem()->childCount(); i++)
            {
                QTreeWidgetItem* item = this->treeWidgetRundown->currentItem()->child(i);

                IRundownWidget* childWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(item, 0))->clone();

                QTreeWidgetItem* childItem = new QTreeWidgetItem();
                parentItem->addChild(childItem);
                this->treeWidgetRundown->setItemWidget(childItem, 0, dynamic_cast<QWidget*>(childWidget));
            }

            delete currentItem;

            this->treeWidgetRundown->setCurrentItem(parentItem);
            this->treeWidgetRundown->doItemsLayout(); // Refresh
        }
    }
    else
    {
        int rowCount = 0;
        if (currentItem != NULL && row > rowCount)
        {
            IRundownWidget* newWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->clone();

            if (parentItem == NULL) // Top level item.
            {
                this->treeWidgetRundown->invisibleRootItem()->takeChild(row);
                this->treeWidgetRundown->invisibleRootItem()->insertChild(row - 1, currentItem);
            }
            else // Group item.
            {
                this->treeWidgetRundown->currentItem()->parent()->takeChild(row);
                this->treeWidgetRundown->currentItem()->parent()->insertChild(row - 1, currentItem);
            }

            this->treeWidgetRundown->setItemWidget(currentItem, 0, dynamic_cast<QWidget*>(newWidget));
            this->treeWidgetRundown->setCurrentItem(currentItem);
            this->treeWidgetRundown->doItemsLayout(); // Refresh
        }
    }

    return true;
}

bool RundownWidget::executeCommand(Playout::PlayoutType::Type type, ActionSource source)
{
    if (this->treeWidgetRundown->currentItem() == NULL)
        return false;

    QWidget* selectedWidget = this->treeWidgetRundown->itemWidget(this->treeWidgetRundown->currentItem(), 0);
    IRundownWidget* rundownWidget = dynamic_cast<IRundownWidget*>(selectedWidget);

    if (source == GpiPulse && !rundownWidget->getCommand()->getAllowGpi())
        return false; // Gpi pulses cannot trigger this item.

    dynamic_cast<IPlayoutCommand*>(selectedWidget)->executeCommand(type);
    if (rundownWidget->isGroup()) // Group
    {
        for (int i = 0; i < this->treeWidgetRundown->currentItem()->childCount(); i++)
        {
            QWidget* childWidget = this->treeWidgetRundown->itemWidget(this->treeWidgetRundown->currentItem()->child(i), 0);
            dynamic_cast<IPlayoutCommand*>(childWidget)->executeCommand(type);
        }
    }

    return true;
}

bool RundownWidget::groupItems()
{
    if (this->treeWidgetRundown->currentItem() == NULL)
        return false;

    QTreeWidgetItem* parentItem = new QTreeWidgetItem();

    RundownGroupWidget* widget = new RundownGroupWidget(LibraryModel(-1, "New group item", "", "GROUP"), this);
    widget->setActive(true);
    widget->setExpanded(true);

    int row = this->treeWidgetRundown->indexOfTopLevelItem(this->treeWidgetRundown->selectedItems().at(0));
    this->treeWidgetRundown->invisibleRootItem()->insertChild(row, parentItem);
    this->treeWidgetRundown->setItemWidget(parentItem, 0, dynamic_cast<QWidget*>(widget));
    this->treeWidgetRundown->expandItem(parentItem);

    foreach (QTreeWidgetItem* item, this->treeWidgetRundown->selectedItems())
    {
        QTreeWidgetItem* childItem = new QTreeWidgetItem();
        parentItem->addChild(childItem);

        IRundownWidget* childWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(item, 0))->clone();
        childWidget->setInGroup(true);
        childWidget->setActive(false);

        this->treeWidgetRundown->setItemWidget(childItem, 0, dynamic_cast<QWidget*>(childWidget));
    }

    removeSelectedItems();

    this->treeWidgetRundown->doItemsLayout(); // Refresh
    this->treeWidgetRundown->setCurrentItem(parentItem);

    return true;
}

bool RundownWidget::ungroupItems()
{
    if (this->treeWidgetRundown->currentItem() == NULL)
        return false;

    QTreeWidgetItem* rootItem = this->treeWidgetRundown->invisibleRootItem();

    if (dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(this->treeWidgetRundown->currentItem(), 0))->isGroup()) // Group.
    {
        QTreeWidgetItem* currentItem = this->treeWidgetRundown->currentItem();
        QTreeWidgetItem* currentItemAbove = this->treeWidgetRundown->itemAbove(this->treeWidgetRundown->currentItem());

        int row = this->treeWidgetRundown->indexOfTopLevelItem(this->treeWidgetRundown->selectedItems().at(0));

        QTreeWidgetItem* newItem = NULL;
        for (int i = 0; i < this->treeWidgetRundown->currentItem()->childCount(); i++)
        {
            QTreeWidgetItem* item = this->treeWidgetRundown->currentItem()->child(i);

            newItem = new QTreeWidgetItem();
            rootItem->insertChild(row + 1, newItem);

            IRundownWidget* newWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(item, 0))->clone();
            newWidget->setInGroup(false);
            newWidget->setActive(false);

            this->treeWidgetRundown->setItemWidget(newItem, 0, dynamic_cast<QWidget*>(newWidget));

            row++;
        }

        this->treeWidgetRundown->setCurrentItem(currentItemAbove);

        delete currentItem;
    }
    else // Group item.
    {
        QTreeWidgetItem* parentItem = this->treeWidgetRundown->currentItem()->parent();

        int parentRow = this->treeWidgetRundown->indexOfTopLevelItem(this->treeWidgetRundown->currentItem()->parent());

        QTreeWidgetItem* newItem = NULL;
        foreach (QTreeWidgetItem* item, this->treeWidgetRundown->selectedItems())
        {
            newItem = new QTreeWidgetItem();
            rootItem->insertChild(parentRow + 1, newItem);

            IRundownWidget* newWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(item, 0))->clone();
            newWidget->setInGroup(false);
            newWidget->setActive(false);

            this->treeWidgetRundown->setItemWidget(newItem, 0, dynamic_cast<QWidget*>(newWidget));

            delete item;

            parentRow++;
        }

        this->treeWidgetRundown->setCurrentItem(newItem);

        if (parentItem->childCount() == 0)
            delete parentItem;
    }

    this->treeWidgetRundown->doItemsLayout(); // Refresh

    return true;
}

void RundownWidget::addBlendModeCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New blend mode item", "", "BLENDMODE")));
}

void RundownWidget::addBrightnessCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New brightness item", "", "BRIGHTNESS")));
}

void RundownWidget::addContrastCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New contrast item", "", "CONTRAST")));
}

void RundownWidget::addCropCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New crop item", "", "CROP")));
}

void RundownWidget::addImageScrollerCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New image scroller item", "", "IMAGESCROLLER")));
}

void RundownWidget::addDeckLinkInputCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New decklink input item", "", "DECKLINKINPUT")));
}

void RundownWidget::addGeometryCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New geometry item", "", "GEOMETRY")));
}

void RundownWidget::addGpiOutputCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New gpi output item", "", "GPIOUTPUT")));
}

void RundownWidget::addFileRecorderCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New file recorder item", "", "FILERECORDER")));
}

void RundownWidget::addGridCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New grid item", "", "GRID")));
}

void RundownWidget::addKeyerCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New keyer item", "", "KEYER")));
}

void RundownWidget::addLevelsCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New levels item", "", "LEVELS")));
}

void RundownWidget::addOpacityCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New opacity item", "", "OPACITY")));
}

void RundownWidget::addSaturationCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New saturation item", "", "SATURATION")));
}

void RundownWidget::addVolumeCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New volume item", "", "VOLUME")));
}

void RundownWidget::addCommitCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New commit item", "", "COMMIT")));
}

void RundownWidget::addAudioCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New media item", "", "AUDIO")));
}

void RundownWidget::addImageCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New media item", "", "STILL")));
}

void RundownWidget::addTemplateCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New media item", "", "TEMPLATE")));
}

void RundownWidget::addVideoCommand()
{
    qApp->postEvent(qApp, new AddRudnownItemEvent(LibraryModel(-1, "New media item", "", "MOVIE")));
}

bool RundownWidget::moveItemOutOfGroup()
{
    if (this->treeWidgetRundown->currentItem()->parent() == NULL) // Top level item.
        return false;

    QTreeWidgetItem* newItem = new QTreeWidgetItem();
    QTreeWidgetItem* currentItem = this->treeWidgetRundown->currentItem();
    QTreeWidgetItem* parentItem = this->treeWidgetRundown->currentItem()->parent(); // Group.
    QTreeWidgetItem* currentItemAbove = this->treeWidgetRundown->itemAbove(this->treeWidgetRundown->currentItem());
    QTreeWidgetItem* parentItemAbove = this->treeWidgetRundown->itemAbove(this->treeWidgetRundown->currentItem()->parent());

    int currentRow  = this->treeWidgetRundown->currentIndex().row();
    int parentRow  = this->treeWidgetRundown->indexOfTopLevelItem(this->treeWidgetRundown->currentItem()->parent());

    IRundownWidget* newWidget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->clone();
    newWidget->setInGroup(false);

    this->treeWidgetRundown->currentItem()->parent()->takeChild(currentRow);
    this->treeWidgetRundown->invisibleRootItem()->insertChild(parentRow + 1, newItem);
    this->treeWidgetRundown->setItemWidget(newItem, 0, dynamic_cast<QWidget*>(newWidget));
    this->treeWidgetRundown->setCurrentItem(newItem);
    this->treeWidgetRundown->doItemsLayout(); // Refresh

    delete currentItem;

    if (parentItem->childCount() == 0)
    {
        this->treeWidgetRundown->setCurrentItem(parentItemAbove);
        delete parentItem;
    }

    return true;
}

bool RundownWidget::moveItemIntoGroup()
{
    if (this->treeWidgetRundown->currentItem()->parent() != NULL) // Group item.
        return false;

    if (dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(this->treeWidgetRundown->currentItem(), 0))->isGroup())
        return false;

    QTreeWidgetItem* currentItemAbove = this->treeWidgetRundown->invisibleRootItem()->child(this->treeWidgetRundown->currentIndex().row() - 1);
    if (currentItemAbove != NULL && dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItemAbove, 0))->isGroup()) // Group.
    {
        QTreeWidgetItem* newItem = new QTreeWidgetItem();
         QTreeWidgetItem* currentItem = this->treeWidgetRundown->currentItem();

        int currentRow  = this->treeWidgetRundown->currentIndex().row();

        IRundownWidget* widget = dynamic_cast<IRundownWidget*>(this->treeWidgetRundown->itemWidget(currentItem, 0))->clone();
        widget->setInGroup(true);

        currentItemAbove->addChild(newItem);

        this->treeWidgetRundown->invisibleRootItem()->takeChild(currentRow);
        this->treeWidgetRundown->setItemWidget(newItem, 0, dynamic_cast<QWidget*>(widget));
        this->treeWidgetRundown->doItemsLayout(); // Refresh
        this->treeWidgetRundown->setCurrentItem(newItem);

        delete currentItem;
    }

    return true;
}

bool RundownWidget::removeSelectedItems()
{
    if (this->treeWidgetRundown->currentItem() == NULL)
        return false;

    QTreeWidgetItem* parentItem = this->treeWidgetRundown->currentItem()->parent();

    foreach (QTreeWidgetItem* item, this->treeWidgetRundown->selectedItems())
       delete item;

    if (parentItem != NULL && parentItem->childCount() == 0)
    {
        this->treeWidgetRundown->setCurrentItem(parentItem);

        delete parentItem;
    }

    checkEmptyRundown();

    return true;
}

void RundownWidget::parsePage(QKeyEvent* keyEvent)
{
    /*if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        return;

    if (keyEvent->key() == Qt::Key_Plus)
    {
        this->page = QString("%1").arg(this->page.toInt() + 1);
        playSelectedEvent();
    }
    else if (keyEvent->key() == Qt::Key_Minus)
    {
        this->page = QString("%1").arg(this->page.toInt() - 1);
        playSelectedEvent();
    }
    else if (keyEvent->key() >= 48 && keyEvent->key() <= 57)
    {
        if (this->isEnterPressed)
        {
            this->isEnterPressed = !this->isEnterPressed;
            this->page = keyEvent->text();
            playSelectedEvent();
        }
        else
            this->page.append(keyEvent->text());
    }*/
}

void RundownWidget::filterRundown()
{
}
