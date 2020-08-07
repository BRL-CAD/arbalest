#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include <Document.h>
#include <QtWidgets/QLabel>
#include <include/QSSPreprocessor.h>
#include <include/Properties.h>
#include <include/Globals.h>
#include <iostream>
#include <QtGui/QtGui>
#include <include/QHBoxWidget.h>
#include <QApplication>
#include <QMessageBox>


using namespace BRLCAD;
using namespace std;



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    loadTheme();
    prepareUi();
    prepareDockables();


    if(QCoreApplication::arguments().length()>1){
        openFile(QString(QCoreApplication::arguments().at(1)));
    }
}

MainWindow::~MainWindow()
{
    for (const std::pair<const int, Document *> pair: documents){
        Document * document = pair.second;
        delete document;
    }
}

void MainWindow::loadTheme()
{
	QFile themeFile( ":themes/arbalest_light.theme" );
	themeFile.open( QFile::ReadOnly );
	QString themeStr( themeFile.readAll() );
	Globals::theme = new QSSPreprocessor(themeStr);
	themeFile.close();

	QFile styleFile( ":styles/arbalest_simple.qss" );
	styleFile.open( QFile::ReadOnly );
	QString styleStr(styleFile.readAll() );
	qApp->setStyleSheet(Globals::theme->process(styleStr));
	styleFile.close();
}

void MainWindow::prepareUi() {
    setWindowFlags(Qt::FramelessWindowHint);    // Hide window title bar

	// Menu bar -------------------------------------------------------------------------------------------------------------
    menuTitleBar = new QMenuBar(this);
    menuTitleBar->installEventFilter(this);
    setMenuBar(menuTitleBar);

    QMenu *fileMenu = menuTitleBar->addMenu(tr("&File"));

    QAction* newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("New .g file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);

    QAction* openAct = new QAction(tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Opens a .g file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFileDialog);
    fileMenu->addAction(openAct);

    QAction* saveAct = new QAction(tr("Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save database"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFileDefaultPath);
    fileMenu->addAction(saveAct);

    QAction* saveAsAct = new QAction(tr("Save As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save database as"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFileDialog);
    fileMenu->addAction(saveAsAct);

    QMenu* editMenu = menuTitleBar->addMenu(tr("&Edit"));

    QMenu* viewMenu = menuTitleBar->addMenu(tr("&View"));

    QAction* autoViewAct = new QAction(tr("Autoview"), this);
    autoViewAct->setShortcuts(QKeySequence::Find);
    autoViewAct->setStatusTip(tr("Autoview based on the current visible objects"));
    connect(autoViewAct, &QAction::triggered, this, [this](){
        documents[activeDocumentId]->getDisplay()->getCamera()->autoview();
    });
    viewMenu->addAction(autoViewAct);

    // Title bar [widgets in the menu bar] ----------------------------------------------------------------------------------------
    QPushButton* applicationIcon = new QPushButton(menuTitleBar);
    applicationIcon->setIcon(QIcon(":/icons/archer.png"));
    applicationIcon->setObjectName("topLeftAppIcon");
    menuTitleBar->setCornerWidget(applicationIcon, Qt::TopLeftCorner);

    QHBoxLayout* layoutTopRightWidget = new QHBoxLayout;
    layoutTopRightWidget->setContentsMargins(0, 0, 0, 0);
    QWidget* topRightWidget = new QWidget;
    topRightWidget->setLayout(layoutTopRightWidget);
    menuTitleBar->setCornerWidget(topRightWidget, Qt::TopRightCorner);
    layoutTopRightWidget->setSpacing(0);

    QPushButton* minimizeButton = new QPushButton(topRightWidget);
    minimizeButton->setIcon(QIcon(":/icons/minimize.png"));
    minimizeButton->setObjectName("minimizeButton");
    connect(minimizeButton, &QPushButton::clicked, this, &MainWindow::minimizeButtonPressed);
    layoutTopRightWidget->addWidget(minimizeButton);


    maximizeButton = new QPushButton(topRightWidget);
    if (this->windowState() == Qt::WindowMaximized) {
        maximizeButton->setIcon(QIcon(":/icons/restore_down.png"));
    }
    else {
        maximizeButton->setIcon(QIcon(":/icons/maximize.png"));
    }
    maximizeButton->setObjectName("maximizeButton");
    connect(maximizeButton, &QPushButton::clicked, this, &MainWindow::maximizeButtonPressed);
    layoutTopRightWidget->addWidget(maximizeButton);

    QPushButton* closeButton = new QPushButton(topRightWidget);
    closeButton->setIcon(QIcon(":/icons/close.png"));
    closeButton->setObjectName("closeButton");
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::closeButtonPressed);
    layoutTopRightWidget->addWidget(closeButton);

	// Status bar ----------------------------------------------------------------------------------------------------------
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBarPathLabel = new QLabel("No document open");
    statusBarPathLabel->setObjectName("statusBarPathLabel");
    statusBar->addWidget(statusBarPathLabel);
	

    // Document area --------------------------------------------------------------------------------------------------------
    documentArea = new QTabWidget(this);
    documentArea->setObjectName("documentArea");
    documentArea->setMovable(true);
    documentArea->setTabsClosable(true);
    setCentralWidget(documentArea);
    documentArea->tabBar()->setObjectName("documentAreaTabBar");
    connect(documentArea, &QTabWidget::currentChanged, this, &MainWindow::onActiveDocumentChanged);
    connect(documentArea, &QTabWidget::tabCloseRequested, this, &MainWindow::tabCloseRequested);
	
    QHBoxWidget * mainTabBarCornerWidget = new QHBoxWidget();
    mainTabBarCornerWidget->setObjectName("mainTabBarCornerWidget");

    QPushButton* newButton = new QPushButton(menuTitleBar);
    newButton->setIcon(QPixmap::fromImage(QImage(":/icons/icons8-new-file-80.png")));
    newButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(newButton);
    connect(newButton, &QPushButton::clicked, this, &MainWindow::newFile);

    QPushButton* openButton = new QPushButton(menuTitleBar);
    openButton->setIcon(QPixmap::fromImage(QImage(":/icons/icons8-opened-folder-80.png")));
    openButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(openButton);	
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFileDialog);

    QPushButton* saveButton = new QPushButton(menuTitleBar);
    saveButton->setIcon(QPixmap::fromImage(QImage(":/icons/icons8-save-80.png")));
    saveButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFileDefaultPath);
	
    QPushButton* saveAsButton = new QPushButton(menuTitleBar);
    saveAsButton->setIcon(QPixmap::fromImage(QImage(":/icons/icons8-save-as-80.png")));
    saveAsButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(saveAsButton);
    connect(saveAsButton, &QPushButton::clicked, this, &MainWindow::saveAsFileDialog);

    documentArea->setCornerWidget(mainTabBarCornerWidget,Qt::Corner::TopLeftCorner);
}

void MainWindow::prepareDockables(){
    // Object tree
    objectTreeWidgetDockable = new Dockable("Objects", this, false, 300);
    addDockWidget(Qt::RightDockWidgetArea, objectTreeWidgetDockable);

    // Properties
    objectPropertiesDockable = new Dockable("Properties", this,true,300);
    addDockWidget(Qt::RightDockWidgetArea, objectPropertiesDockable);

    // Toolbox
    toolboxDockable = new Dockable("Make", this,true,30);
    toolboxDockable->hideHeader();
    addDockWidget(Qt::LeftDockWidgetArea, toolboxDockable);
}

void MainWindow::newFile() {
    Document* document = new Document(documentsCount);
    document->getObjectTreeWidget()->setObjectName("dockableContent");
    document->getProperties()->setObjectName("dockableContent");
    documents[documentsCount++] = document;
    QString filename( "Untitled");
    const int tabIndex = documentArea->addTab(document->getDisplay(), filename);
    documentArea->setCurrentIndex(tabIndex);
    connect(documents[activeDocumentId]->getObjectTreeWidget(), &ObjectTreeWidget::selectionChanged,
            this, &MainWindow::objectTreeWidgetSelectionChanged);
    
}

void MainWindow::openFile(const QString& filePath) {
    Document* document = nullptr;

    try {
        document = new Document(documentsCount, &filePath);
    }
    catch (...) {
        QString msg = "Failed to open " + filePath;
        statusBar->showMessage(msg, statusBarShortMessageDuration);

        QMessageBox msgBox;
        msgBox.setText(msg);
        msgBox.exec();
    }

    if (document != nullptr) {
        document->getObjectTreeWidget()->setObjectName("dockableContent");
        document->getProperties()->setObjectName("dockableContent");
        documents[documentsCount++] = document;
        QString filename(QFileInfo(filePath).fileName());
        const int tabIndex = documentArea->addTab(document->getDisplay(), filename);
        documentArea->setCurrentIndex(tabIndex);
        connect(documents[activeDocumentId]->getObjectTreeWidget(), &ObjectTreeWidget::selectionChanged,
                this, &MainWindow::objectTreeWidgetSelectionChanged);
    }
}

bool MainWindow::saveFile(const QString& filePath) {
    return documents[activeDocumentId]->getDatabase()->Save(filePath.toUtf8().data());
}

void MainWindow::openFileDialog()
{
	const QString filePath = QFileDialog::getOpenFileName(documentArea, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()){
        openFile(filePath);
    }
}

void MainWindow::saveAsFileDialog() {
	const QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()) {
        if (saveFile(filePath))
        {
            documents[activeDocumentId]->setFilePath(filePath);
            QString filename(QFileInfo(filePath).fileName());
            documentArea->setTabText(documentArea->currentIndex(), filename);
            statusBarPathLabel->setText(*documents[activeDocumentId]->getFilePath());
            statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
        }
    }
}

void MainWindow::saveFileDefaultPath() {
    if (documents[activeDocumentId]->getFilePath() == nullptr) saveAsFileDialog();
    else {
        const QString filePath = *documents[activeDocumentId]->getFilePath();
        if (!filePath.isEmpty()) {
            if (saveFile(filePath))
            {
                statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
            }
        }
    }
}

void MainWindow::onActiveDocumentChanged(const int newIndex){
    Display * display = dynamic_cast<Display*>(documentArea->widget(newIndex));
    if (display == nullptr) return;
    if (display->getDocument()->getDocumentId() != activeDocumentId){
        activeDocumentId = display->getDocument()->getDocumentId();
        objectTreeWidgetDockable->setContent(documents[activeDocumentId]->getObjectTreeWidget());
        objectPropertiesDockable->setContent(documents[activeDocumentId]->getProperties());
        statusBarPathLabel->setText(documents[activeDocumentId]->getFilePath()  != nullptr ? *documents[activeDocumentId]->getFilePath() : "Untitled");
    }
}

void MainWindow::tabCloseRequested(const int i) const
{
    documentArea->removeTab(i);
    if (documentArea->currentIndex() == -1){
        objectTreeWidgetDockable->clear();
        objectPropertiesDockable->clear();
    }
}

void MainWindow::objectTreeWidgetSelectionChanged(int objectId) {
    documents[activeDocumentId]->getProperties()->bindObject(objectId);
}

void MainWindow::closeButtonPressed(){
    close();
}

void MainWindow::minimizeButtonPressed() {
    showMinimized();
}

void MainWindow::maximizeButtonPressed() {
    if(!isMaximized())showMaximized();
    else showNormal();
}

// drag window by holding from menu bar (now functioning as title bar too)
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    static QPoint dragPosition{};
    if (watched == menuTitleBar)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::LeftButton)
            {
                dragPosition = mouse_event->globalPos() - frameGeometry().topLeft();
                return false;
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if (mouse_event->buttons() & Qt::LeftButton)
            {
                if(isMaximized()) return false;//showNormal();
                //todo showNormal when dragged
                move(mouse_event->globalPos() - dragPosition);
                return false;
            }
        }


    }
    return false;
}

void MainWindow::changeEvent( QEvent* e ) {
    if (e->type() == QEvent::WindowStateChange) {
        if (this->windowState() == Qt::WindowMaximized) {
            maximizeButton->setIcon(QIcon(":/icons/restore_down.png"));
        }
        else{
            maximizeButton->setIcon(QIcon(":/icons/maximize.png"));
        }
    }
}