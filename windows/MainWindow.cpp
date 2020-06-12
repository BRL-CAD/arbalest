#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include <ArbalestGlobals.h>
#include <Display.h>
#include "ui_MainWindow.h"
#include "GraphicsViewOpenGL.h"

using namespace BRLCAD;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMaximized();

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveAsFile);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
//    for(const std::string& item :ArbalestGlobals::databaseManager.getObjectsList(currentDatabaseIndex)){
//        ui->objectsTree->addItem( QString::fromStdString(item));
//    }
    auto doc = Document(filePath.toUtf8().data());

    ui->documentArea->addSubWindow(doc.getDisplay());

}


void MainWindow::saveAsFile(){
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    //ArbalestGlobals::databaseManager.getDatabase(0)->Save(filePath.toUtf8().data());
}
