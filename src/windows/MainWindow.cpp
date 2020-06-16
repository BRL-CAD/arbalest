#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include <ArbalestGlobals.h>
#include <Display.h>
#include <iostream>
#include "ui_MainWindow.h"
#include "GraphicsViewOpenGL.h"

using namespace std;
using namespace BRLCAD;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMaximized();

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFileDialog);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveAsFile);

    documentArea = new QMdiArea();
    setCentralWidget(documentArea);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile(const QString& filePath){
    auto doc = Document(filePath.toUtf8().data());

    ui->dockWidgetObjectsTree->setWidget(doc.getObjectsTree());

    documentArea->addSubWindow(doc.getWindow());
    doc.getWindow()->show();
}
void MainWindow::openFileDialog()
{
    auto filePath = QFileDialog::getOpenFileName(documentArea, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    openFile(filePath);
}


void MainWindow::saveAsFile(){
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    //database->Save(filePath.toUtf8().data());
}
