#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include <ArbalestGlobals.h>
#include "ui_mainwindow.h"
#include "GraphicsViewOpenGL.h"
#include "../display/DisplayService.cpp"


GraphicsViewOpenGL * gv;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMaximized();

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveAsFile);

    gv = new GraphicsViewOpenGL();
    gv->setFocus();
    gv->setFocusPolicy(Qt::ClickFocus);
    setCentralWidget(gv);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    currentDatabaseIndex = ArbalestGlobals::databaseManager.addNewDatabase(filePath.toUtf8().data());
    auto gom = ArbalestGlobals::databaseManager.getGeometryOperationsManager(currentDatabaseIndex);
    printf("s");
    gom->createSphere();
    printf("g");
    gv->vp=DisplayService::getVList(ArbalestGlobals::databaseManager.getDatabase(currentDatabaseIndex));
    gv->update();

    for(const std::string& item :ArbalestGlobals::databaseManager.getObjectsList(currentDatabaseIndex)){
        ui->objectsTree->addItem( QString::fromStdString(item));
    }
}

void MainWindow::saveAsFile(){
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    ArbalestGlobals::databaseManager.getDatabase(0)->Save(filePath.toUtf8().data());
}