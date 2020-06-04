#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include "../include/Globals.h"
#include "ui_mainwindow.h"
#include "../display/GraphicsViewOpenGL.h"
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
    currentDatabaseIndex = Globals::databaseManager.addNewDatabase(filePath.toUtf8().data());

    gv->vp=DisplayService::getVList(Globals::databaseManager.getDatabase(currentDatabaseIndex));
    gv->update();

    for(std::string item :Globals::databaseManager.getObjectsList(currentDatabaseIndex)){
        ui->objectsTree->addItem( QString::fromStdString(item));
    }
}

void MainWindow::saveAsFile(){
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    Globals::databaseManager.getDatabase(0)->Save(filePath.toUtf8().data());
}