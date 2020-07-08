#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>
#include "MainWindow.h"
#include <Document.h>
#include <SubWindow.h>
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
    connect(ui->documentArea, &QMdiArea::subWindowActivated, this, &MainWindow::onActiveDocumentChanged);
}

MainWindow::~MainWindow()
{
    for (std::pair<const int, Document *> pair: documents){
        Document * document = pair.second;
        delete document;
    }

    delete ui;
}

void MainWindow::onActiveDocumentChanged(QMdiSubWindow * window){
    SubWindow * subWindow = dynamic_cast<SubWindow*>(window);

    if (subWindow && subWindow->getDocumentId() != activeDocumentId){
        activeDocumentId = subWindow->getDocumentId();
        ui->dockWidgetObjectsTree->setWidget(documents[activeDocumentId]->getObjectsTree());
    }
}

void MainWindow::openFile(const QString& filePath){
    Document & document = * (new Document(filePath.toUtf8().data(), documentsCount));
    documents[documentsCount++] = &document;

    ui->documentArea->addSubWindow(document.getWindow());
    document.getWindow()->show();

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

    // Hiden Window Title
    setWindowFlags(Qt::FramelessWindowHint);
    ui->documentArea->setBackground(QBrush(QColor("#FDFDFD")));

    ui->documentArea->setDocumentMode(true);

    QLabel * applicationIcon = new QLabel;
    applicationIcon->setPixmap(QPixmap (":/icons/archer.png").scaledToWidth(17));
    applicationIcon->setScaledContents(true);
    menuBar()->setCornerWidget(applicationIcon, Qt::TopLeftCorner);


    QPushButton* closeButton = new QPushButton( menuBar());
    closeButton->setIcon(QIcon(":/icons/close.png"));
    closeButton->setObjectName("closeButton");
    connect(closeButton,  &QPushButton::clicked, this, &MainWindow::closeButtonPressed);
    menuBar()->setCornerWidget(closeButton, Qt::TopRightCorner);


    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);

    // Load an application style
    QFile styleFile( ":styles/arbalest_light.qss" );
    styleFile.open( QFile::ReadOnly );
    QString style( styleFile.readAll() );
    this->setStyleSheet( style );
    styleFile.close();
}

void MainWindow::closeButtonPressed(){
    close();
}