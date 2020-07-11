#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>
#include "MainWindow.h"
#include <Document.h>
#include <QtWidgets/QLabel>
#include "ui_MainWindow.h"

using namespace BRLCAD;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setTheme();
    showMaximized();

    ui->dockWidgetObjectsTree->setWidget(new QTreeView()); //todo: free this

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFileDialog);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveFileDialog);
    connect(ui->documentArea, &QTabWidget::currentChanged, this, &MainWindow::onActiveDocumentChanged);
    connect(ui->documentArea, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
    ui->menubar->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    for (std::pair<const int, Document *> pair: documents){
        Document * document = pair.second;
        delete document;
    }

    delete ui;
}


void MainWindow::onActiveDocumentChanged(int newIndex){
    Display * display = dynamic_cast<Display*>(ui->documentArea->widget(newIndex));
    if (display == nullptr) return;
    if (display->getDocumentId() != activeDocumentId){
        activeDocumentId = display->getDocumentId();
        ui->dockWidgetObjectsTree->setWidget(documents[activeDocumentId]->getObjectsTree());
        statusBarPathLabel->setText(documents[activeDocumentId]->getFilePath());
    }
}

void MainWindow::tabCloseRequested(int i){
    ui->documentArea->removeTab(i);
}

void MainWindow::openFile(const QString& filePath){
    Document & document = * (new Document(filePath.toUtf8().data(), documentsCount));
    documents[documentsCount++] = &document;

    QString filename(QFileInfo(filePath).fileName());

    ui->documentArea->addTab(document.getDisplay(),filename);
}

void MainWindow::openFileDialog()
{
    QString filePath = QFileDialog::getOpenFileName(ui->documentArea, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()){
        openFile(filePath);
    }
}

void MainWindow::saveFileDialog(){
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    //database->Save(filePath.toUtf8().data());
}

void MainWindow::setTheme() {

    // Hide window title bar
    setWindowFlags(Qt::FramelessWindowHint);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    ui->documentArea->setContentsMargins(0,0,0,0);

    QPushButton* applicationIcon = new QPushButton( menuBar());
    applicationIcon->setIcon(QIcon(":/icons/archer.png"));
    applicationIcon->setObjectName("applicationIcon");
    menuBar()->setCornerWidget(applicationIcon, Qt::TopLeftCorner);

    QHBoxLayout *layoutTopRightWidget = new QHBoxLayout;
    layoutTopRightWidget->setContentsMargins(0,0,0,0);
    QWidget * topRightWidget = new QWidget;
    topRightWidget->setLayout(layoutTopRightWidget);
    menuBar()->setCornerWidget(topRightWidget, Qt::TopRightCorner);
    layoutTopRightWidget->setSpacing(0);

    statusBarPathLabel = new QLabel("No document open");
    statusBarPathLabel->setObjectName("statusBarPathLabel");
    statusBar()->addWidget(statusBarPathLabel);

    QPushButton* minimizeButton = new QPushButton( topRightWidget);
    minimizeButton->setIcon(QIcon(":/icons/minimize.png"));
    minimizeButton->setObjectName("minimizeButton");
    connect(minimizeButton,  &QPushButton::clicked, this, &MainWindow::minimizeButtonPressed);
    layoutTopRightWidget->addWidget(minimizeButton);

    maximizeButton = new QPushButton( topRightWidget);
    maximizeButton->setIcon(QIcon(":/icons/restore_down.png"));
    maximizeButton->setObjectName("maximizeButton");
    connect(maximizeButton,  &QPushButton::clicked, this, &MainWindow::maximizeButtonPressed);
    layoutTopRightWidget->addWidget(maximizeButton);

    QPushButton* closeButton = new QPushButton( topRightWidget);
    closeButton->setIcon(QIcon(":/icons/close.png"));
    closeButton->setObjectName("closeButton");
    connect(closeButton,  &QPushButton::clicked, this, &MainWindow::closeButtonPressed);
    layoutTopRightWidget->addWidget(closeButton);

    QLabel * objectsTreeLabel = new QLabel("Objects");
    objectsTreeLabel->setObjectName("objectsTreeLabel");
    ui->dockWidgetObjectsTree->setTitleBarWidget(objectsTreeLabel);

    // Load an application style
    QFile styleFile( ":styles/arbalest_light.qss" );
    styleFile.open( QFile::ReadOnly );
    QString style( styleFile.readAll() );
    qApp->setStyleSheet( style );
    styleFile.close();
}

void MainWindow::closeButtonPressed(){
    close();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    static QPoint dragPosition{};
    if (watched == ui->menubar)
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

void MainWindow::minimizeButtonPressed() {
    showMinimized();
}

void MainWindow::maximizeButtonPressed() {
    if(!isMaximized())showMaximized();
    else showNormal();
}
