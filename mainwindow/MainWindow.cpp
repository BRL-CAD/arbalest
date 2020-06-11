#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include <ArbalestGlobals.h>
#include <Display.h>
#include "ui_MainWindow.h"
#include "GraphicsViewOpenGL.h"
#include "../display/DisplayService.cpp"

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

    auto doc = Document(filePath.toUtf8().data());

    ui->documentArea->addSubWindow(doc.getDisplay());

//    currentDatabaseIndex = ArbalestGlobals::databaseManager.addNewDatabase(filePath.toUtf8().data());
//    auto gom = ArbalestGlobals::databaseManager.getGeometryOperationsManager(currentDatabaseIndex);
//    gom->createSphere();
//    //gv->vp=DisplayService::getVList(ArbalestGlobals::databaseManager.getDatabase(currentDatabaseIndex));
//    gv->update();
//
//    for(const std::string& item :ArbalestGlobals::databaseManager.getObjectsList(currentDatabaseIndex)){
//        ui->objectsTree->addItem( QString::fromStdString(item));
//    }
//
//    auto h = ArbalestGlobals::databaseManager.getVectorLists(currentDatabaseIndex);
//    std::string g = "sph";
//    auto m = h[g];
//    auto j = m[3];
//    int t;
//    t++;

}


void MainWindow::saveAsFile(){
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    //ArbalestGlobals::databaseManager.getDatabase(0)->Save(filePath.toUtf8().data());
}
