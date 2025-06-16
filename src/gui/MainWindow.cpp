#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include "ViewportGrid.h"
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
#include <QComboBox>
#include <include/RaytraceView.h>
#include <include/AboutWindow.h>
#include <include/HelpWidget.h>
#include <brlcad/Database/Arb8.h>
#include <QtWidgets/QtWidgets>
#include <brlcad/Database/Torus.h>
#include <brlcad/Database/Cone.h>
#include <brlcad/Database/Particle.h>
#include <brlcad/Database/Paraboloid.h>
#include <brlcad/Database/Hyperboloid.h>
#include <brlcad/Database/Halfspace.h>
#include <brlcad/Database/EllipticalTorus.h>
#include <brlcad/Database/Ellipsoid.h>
#include <brlcad/Database/HyperbolicCylinder.h>
#include <brlcad/Database/ParabolicCylinder.h>
#include <include/MatrixTransformWidget.h>
#include "MoveCameraMouseAction.h"
#include "SelectMouseAction.h"

using namespace BRLCAD;
using namespace std;



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_mouseAction{nullptr} {
    loadTheme();
    prepareUi();
    setIcons();
    prepareDockables();

    documentArea->addTab(new HelpWidget(this), "Quick Start");
    if (QCoreApplication::arguments().length() > 1) {
        openFile(QString(QCoreApplication::arguments().at(1)));
    }
    Globals::mainWindow = this;
}

MainWindow::~MainWindow() {
    for (const std::pair<const int, Document*>& pair : documents) {
        Document* document = pair.second;
        delete document;
    }
}

void MainWindow::loadTheme() {
    QSettings settings("BRLCAD", "arbalest");
    int themeIndex = settings.value("themeIndex", 0).toInt();

    QStringList themes = {":themes/arbalest_light.theme", ":themes/arbalest_dark.theme"};

	QFile themeFile(themes[themeIndex]);
	themeFile.open(QFile::ReadOnly);
	QString themeStr(themeFile.readAll());
	Globals::theme = new QSSPreprocessor(themeStr);
	themeFile.close();

	QFile styleFile(":styles/arbalest_simple.qss");
	styleFile.open(QFile::ReadOnly);
	QString styleStr(styleFile.readAll());
	qApp->setStyleSheet(Globals::theme->process(styleStr));
	styleFile.close();
}

void MainWindow::prepareUi() {
    setWindowTitle("Arbalest");
    setWindowIcon(QIcon(":/icons/arbalest_icon.png"));
    

    // ---------- Menu bar ----------
    menuTitleBar = new QMenuBar(this);
    setMenuBar(menuTitleBar);

    // File menu
    QMenu *fileMenu = menuTitleBar->addMenu(tr("&File"));

    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("New .g file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Opens a .g file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFileDialog);
    fileMenu->addAction(openAct);

    saveAct = new QAction(tr("Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save database"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFileDefaultPath);
    fileMenu->addAction(saveAct);

    saveAsAct = new QAction(tr("Save As..."), this);
    saveAsAct->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    saveAsAct->setStatusTip(tr("Save database as"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFileDialog);
    fileMenu->addAction(saveAsAct);
    
    fileMenu->addSeparator();

    quitAct = new QAction(tr("Quit"), this);
    quitAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
    quitAct->setStatusTip(tr("Quit"));
    connect(quitAct, &QAction::triggered, this, [this]() {
        QCoreApplication::quit();
    });
    fileMenu->addAction(quitAct);

    // Create menu
    QMenu* createMenu = menuTitleBar->addMenu(tr("&Create"));

    QAction* createArb8Act = new QAction(tr("Arb8"), this);
    connect(createArb8Act, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Arb8 * object = new BRLCAD::Arb8();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createArb8Act);

    QAction* createConeAct = new QAction(tr("Cone"), this);
    connect(createConeAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Cone * object = new BRLCAD::Cone();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createConeAct);

    QAction* createEllipsoidAct = new QAction(tr("Ellipsoid"), this);
    connect(createEllipsoidAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Ellipsoid * object = new BRLCAD::Ellipsoid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createEllipsoidAct);

    QAction* createEllipticalTorusAct = new QAction(tr("Elliptical Torus"), this);
    connect(createEllipticalTorusAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::EllipticalTorus * object = new BRLCAD::EllipticalTorus();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createEllipticalTorusAct);

    QAction* createHalfspaceAct = new QAction(tr("Halfspace"), this);
    connect(createHalfspaceAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Halfspace * object = new BRLCAD::Halfspace();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createHalfspaceAct);

    QAction* createHyperbolicCylinderAct = new QAction(tr("Hyperbolic Cylinder"), this);
    connect(createHyperbolicCylinderAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::HyperbolicCylinder * object = new BRLCAD::HyperbolicCylinder();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createHyperbolicCylinderAct);

    QAction* createHyperboloidAct = new QAction(tr("Hyperboloid"), this);
    connect(createHyperboloidAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Hyperboloid * object = new BRLCAD::Hyperboloid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createHyperboloidAct);

    QAction* createParabolicCylinderAct = new QAction(tr("Parabolic Cylinder"), this);
    connect(createParabolicCylinderAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::ParabolicCylinder * object = new BRLCAD::ParabolicCylinder();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createParabolicCylinderAct);

    QAction* createParaboloidAct = new QAction(tr("Paraboloid"), this);
    connect(createParaboloidAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Paraboloid * object = new BRLCAD::Paraboloid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createParaboloidAct);

    QAction* createParticleAct = new QAction(tr("Particle"), this);
    connect(createParticleAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Particle * object = new BRLCAD::Particle();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createParticleAct);

    QAction* createTorusAct = new QAction(tr("Torus"), this);
    connect(createTorusAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Torus * object = new BRLCAD::Torus();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId, true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getViewportGrid()->forceRerenderAllViewports();
    });
    createMenu->addAction(createTorusAct);

    // Edit menu
    QMenu* editMenu = menuTitleBar->addMenu(tr("&Edit"));

    QAction* relativeMoveAct = new QAction("Relative move selected object", this);
    relativeMoveAct->setStatusTip(tr("Relative move selected object. Top objects cannot be moved."));
    connect(relativeMoveAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Translate);
    });
    editMenu->addAction(relativeMoveAct);

    QAction* relativeScaleAct = new QAction("Relative scale selected object", this);
    relativeScaleAct->setStatusTip(tr("Relative scale selected object. Top objects cannot be scaled."));
    connect(relativeScaleAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Scale);
    });
    editMenu->addAction(relativeScaleAct);

    QAction* relativeRotateAct = new QAction("Relative rotate selected object", this);
    relativeRotateAct->setStatusTip(tr("Relative rotate selected object. Top objects cannot be rotated."));
    connect(relativeRotateAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Rotate);
    });
    editMenu->addAction(relativeRotateAct);

    selectObjectAct = new QAction(tr("Select object"), this);
    selectObjectAct->setStatusTip(tr("Select object."));
    selectObjectAct->setCheckable(true);
    connect(selectObjectAct, &QAction::toggled, this, &MainWindow::updateMouseButtonObjectState);
    editMenu->addAction(selectObjectAct);

    // View menu
    QMenu* viewMenu = menuTitleBar->addMenu(tr("&View"));

    QAction* resetViewportAct = new QAction("Reset current viewport", this);
    resetViewportAct->setStatusTip(tr("Reset to default camera orientation for the viewport and autoview to currently visible objects"));
    connect(resetViewportAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getViewportGrid()->resetViewPort(documents[activeDocumentId]->getViewportGrid()->getActiveViewportId());
    });
    viewMenu->addAction(resetViewportAct);

    resetAllViewportsAct = new QAction("Reset all viewports", this);
    resetAllViewportsAct->setStatusTip(tr("Reset to default camera orientation for each viewport and autoview to visible objects"));
    connect(resetAllViewportsAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getViewportGrid()->resetAllViewPorts();
    });
    viewMenu->addAction(resetAllViewportsAct);

    viewMenu->addSeparator();

    autoViewAct = new QAction(tr("Focus visible objects (all viewports)"), this);
    autoViewAct->setShortcut(Qt::Key_F|Qt::CTRL);
    autoViewAct->setStatusTip(tr("Resize and center the view based on the current visible objects"));
    connect(autoViewAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        for(Viewport * display : documents[activeDocumentId]->getViewportGrid()->getViewports()) {
            display->getCamera()->autoview();
            display->forceRerenderFrame();
        }
    });
    viewMenu->addAction(autoViewAct);

    QAction* autoViewSingleAct = new QAction(tr("Focus visible objects (current viewport)"), this);
    autoViewSingleAct->setStatusTip(tr("Resize and center the view based on the current visible objects"));
    connect(autoViewSingleAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getViewport()->getCamera()->autoview();
        documents[activeDocumentId]->getViewport()->forceRerenderFrame();
    });
    viewMenu->addAction(autoViewSingleAct);

    centerViewAct = new QAction(tr("Focus selected object"), this);
    centerViewAct->setStatusTip(tr("Resize and center the view based on the selected objects"));
    centerViewAct->setShortcut(Qt::Key_F);
    connect(centerViewAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        documents[activeDocumentId]->getViewport()->getCamera()->centerView(objectId);
    });
    viewMenu->addAction(centerViewAct);
    
    viewMenu->addSeparator();

    currentViewport = new QComboBox();

    QMenu* singleView = viewMenu->addMenu(tr("&Single View"));

    QActionGroup *viewportMenuGroup = new QActionGroup(this);
    for(int i=0;i<4;i++) {
        singleViewAct[i] = new QAction("Viewport " + QString::number(i+1), this);
        viewportMenuGroup->addAction(singleViewAct[i]);
        singleViewAct[i]->setCheckable(true);
        singleViewAct[i]->setStatusTip("Viewport viewport " + QString::number(i+1));
        connect(singleViewAct[i], &QAction::triggered, this, [this,i]() {
            if (activeDocumentId == -1) return;
            documents[activeDocumentId]->getViewportGrid()->singleViewportMode(i);
            currentViewport->setCurrentIndex(i);
        });
        singleView->addAction(singleViewAct[i]);
    }
    singleViewAct[0]->setShortcut(Qt::Key_1);
    singleViewAct[1]->setShortcut(Qt::Key_2);
    singleViewAct[2]->setShortcut(Qt::Key_3);
    singleViewAct[3]->setShortcut(Qt::Key_4);
    singleViewAct[3]->setChecked(true);

    QAction* quadViewAct = new QAction(tr("All Viewports"), this);
    quadViewAct->setStatusTip(tr("Viewport 4 viewports"));
    connect(quadViewAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getViewportGrid()->quadViewportMode();
        for(QAction *i : singleViewAct) i->setChecked(false);
        currentViewport->setCurrentIndex(4);
    });
    quadViewAct->setShortcut(Qt::Key_5);
    viewMenu->addAction(quadViewAct);

    viewMenu->addSeparator();
    
    toggleGridAct = new QAction(tr("Toggle grid on/off"), this);
    toggleGridAct->setCheckable(true);
    toggleGridAct->setShortcut(Qt::Key_G);
    connect(toggleGridAct, &QAction::toggled, this, [=]() {
        if (activeDocumentId == -1) {
            toggleGridAct->setChecked(false);
            return;
        }

        if (documents[activeDocumentId]->getViewportGrid()->getActiveViewport()->gridEnabled == false) {
            documents[activeDocumentId]->getViewportGrid()->getActiveViewport()->gridEnabled = true;
            toggleGridAct->setToolTip("Toggle grid OFF (G)");
        }
        else {
            documents[activeDocumentId]->getViewportGrid()->getActiveViewport()->gridEnabled = false;
            toggleGridAct->setToolTip("Toggle grid ON (G)");
        }

        documents[activeDocumentId]->getViewportGrid()->getActiveViewport()->forceRerenderFrame();
    });
    viewMenu->addAction(toggleGridAct);

    QMenu* selectThemeAct = viewMenu->addMenu(tr("Select theme"));
    QActionGroup *selectThemeActGroup = new QActionGroup(this);

    themeAct[0] = new QAction(tr("Arbalest Light"), this);
    themeAct[0]->setCheckable(true);
    connect(themeAct[0], &QAction::triggered, this, [this](){
        QSettings settings("BRLCAD", "arbalest");
        settings.setValue("themeIndex", 0);
        QMessageBox::information(this, "Application Restart Needed", "Selected theme will be set after next restart of Arbalest", QMessageBox::Ok);
    });
    selectThemeActGroup->addAction(themeAct[0]);
    selectThemeAct->addAction(themeAct[0]);

    themeAct[1] = new QAction(tr("Arbalest Dark"), this);
    themeAct[1]->setCheckable(true);
    connect(themeAct[1], &QAction::triggered, this, [this](){
        QSettings settings("BRLCAD", "arbalest");
        settings.setValue("themeIndex", 1);
        QMessageBox::information(this, "Application Restart Needed", "Selected theme will be set after next restart of Arbalest", QMessageBox::Ok);

    });
    selectThemeActGroup->addAction(themeAct[1]);
    selectThemeAct->addAction(themeAct[1]);

    QSettings settings("BRLCAD", "arbalest");
    themeAct[settings.value("themeIndex",0).toInt()]->setChecked(true);

    // Raytrace menu
    QMenu* raytrace = menuTitleBar->addMenu(tr("&Raytrace"));

    raytraceAct = new QAction(tr("Raytrace current viewport"), this);
    raytraceAct->setStatusTip(tr("Raytrace current viewport"));
    raytraceAct->setShortcut(Qt::CTRL|Qt::Key_R);
    connect(raytraceAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        statusBar->showMessage("Raytracing current viewport...", statusBarShortMessageDuration);
        QCoreApplication::processEvents();
        documents[activeDocumentId]->getRaytraceWidget()->raytrace();
        statusBar->showMessage("Raytracing completed.", statusBarShortMessageDuration);
    });
    raytrace->addAction(raytraceAct);

    QAction* setRaytraceBackgroundColorAct = new QAction(tr("Set raytrace background color.."), this);
    connect(setRaytraceBackgroundColorAct, &QAction::triggered, this, [this](){
        QSettings settings("BRLCAD", "arbalest");
        QColor color=settings.value("raytraceBackground").value<QColor>();
        QColor selectedColor = QColorDialog::getColor(color);
        settings.setValue("raytraceBackground", selectedColor);
    });
    raytrace->addAction(setRaytraceBackgroundColorAct);

    // Help menu
    QMenu* help = menuTitleBar->addMenu(tr("&Help"));

    QAction* aboutAct = new QAction(tr("About"), this);
    connect(aboutAct, &QAction::triggered, this, [this](){
        (new AboutWindow())->show();
    });
    help->addAction(aboutAct);

    QAction* helpAct = new QAction(tr("Help"), this);
    helpAct->setShortcut(Qt::Key_F1);
    connect(helpAct, &QAction::triggered, this, [this](){
        HelpWidget * helpWidget = dynamic_cast<HelpWidget*>(documentArea->widget(0));
        if (helpWidget== nullptr){
            documentArea->insertTab(0,new HelpWidget(this), "Quick Start");
        }
        documentArea->setCurrentIndex(0);
    });
    help->addAction(helpAct);
    
    
    // ---------- Status bar ----------
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBarPathLabel = new QLabel("No document open");
    statusBarPathLabel->setObjectName("statusBarPathLabel");
    statusBar->addWidget(statusBarPathLabel);


    // ---------- Document area ----------
    documentArea = new QTabWidget(this);
    documentArea->setObjectName("documentArea");
    documentArea->setMovable(true);
    documentArea->setTabsClosable(true);
    setCentralWidget(documentArea);
    documentArea->tabBar()->setObjectName("documentAreaTabBar");
    connect(documentArea, &QTabWidget::currentChanged, this, &MainWindow::onActiveDocumentChanged);
    connect(documentArea, &QTabWidget::tabCloseRequested, this, &MainWindow::tabCloseRequested);
    connect(documentArea, &QTabWidget::currentChanged, this, &MainWindow::updateMouseButtonObjectState);

    QHBoxWidget * mainTabBarCornerWidget = new QHBoxWidget();
    mainTabBarCornerWidget->setObjectName("mainTabBarCornerWidget");

    QToolButton* newButton = new QToolButton(menuTitleBar);
    newButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    newButton->setDefaultAction(newAct);
    newButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(newButton);
    
    QToolButton* openButton = new QToolButton(menuTitleBar);
    openButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    openButton->setDefaultAction(openAct);
    openButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(openButton);

    QToolButton* saveButton = new QToolButton(menuTitleBar);
    saveButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    saveButton->setDefaultAction(saveAct);
    saveButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(saveButton);

    QToolButton* saveAsButton = new QToolButton(menuTitleBar);
    saveAsButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    saveAsButton->setDefaultAction(saveAsAct);
    saveAsButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(saveAsButton);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    QToolButton* focusAll = new QToolButton(menuTitleBar);
    focusAll->setToolButtonStyle(Qt::ToolButtonIconOnly);
    focusAll->setDefaultAction(autoViewAct);
    focusAll->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(focusAll);

    QToolButton* focusCurrent = new QToolButton(menuTitleBar);
    focusCurrent->setToolButtonStyle(Qt::ToolButtonIconOnly);
    focusCurrent->setDefaultAction(centerViewAct);
    focusCurrent->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(focusCurrent);

    QToolButton* resetViewports = new QToolButton(menuTitleBar);
    resetViewports->setToolButtonStyle(Qt::ToolButtonIconOnly);
    resetViewports->setDefaultAction(resetAllViewportsAct);
    resetViewports->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(resetViewports);
    
    currentViewport->setToolTip("Change viewport");
    currentViewport->addItem("Viewport 1");
    currentViewport->addItem("Viewport 2");
    currentViewport->addItem("Viewport 3");
    currentViewport->addItem("Viewport 4");
    currentViewport->addItem("All Viewports");
    currentViewport->setCurrentIndex(3);
    connect(currentViewport, QOverload<int>::of(&QComboBox::activated),[=](int index){
        if (activeDocumentId == -1) return;
        if (index <4) documents[activeDocumentId]->getViewportGrid()->singleViewportMode(index);
        else documents[activeDocumentId]->getViewportGrid()->quadViewportMode();
        for(QAction *i : singleViewAct) i->setChecked(false);
        if(index != 4) singleViewAct[index]->setChecked(true);
    });
    mainTabBarCornerWidget->addWidget(currentViewport);

    QToolButton* toggleGrid = new QToolButton(menuTitleBar);
    toggleGrid->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toggleGrid->setDefaultAction(toggleGridAct);
    toggleGrid->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(toggleGrid);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    QToolButton* selectObjectButton = new QToolButton(menuTitleBar);
    selectObjectButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    selectObjectButton->setDefaultAction(selectObjectAct);
    selectObjectButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(selectObjectButton);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    QToolButton* raytraceButton = new QToolButton(menuTitleBar);
    raytraceButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    raytraceButton->setDefaultAction(raytraceAct);
    raytraceButton->setObjectName("toolbarButton");
    mainTabBarCornerWidget->addWidget(raytraceButton);

    documentArea->setCornerWidget(mainTabBarCornerWidget,Qt::Corner::TopRightCorner);
}

void MainWindow::setIcons() {
    // File menu
    QIcon newActIcon;
    newActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-NewActIcon")), QIcon::Normal);
    newActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-NewActIcon-Active")), QIcon::Active);
    newAct->setIcon(newActIcon);

    QIcon openActIcon;
    openActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-OpenActIcon")), QIcon::Normal);
    openActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-OpenActIcon-Active")), QIcon::Active);
    openAct->setIcon(openActIcon);

    QIcon saveActIcon;
    saveActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-SaveActIcon")), QIcon::Normal);
    saveActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-SaveActIcon-Active")), QIcon::Active);
    saveAct->setIcon(saveActIcon);

    QIcon saveAsActIcon;
    saveAsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-SaveAsActIcon")), QIcon::Normal);
    saveAsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-SaveAsActIcon-Active")), QIcon::Active);
    saveAsAct->setIcon(saveAsActIcon);

    QIcon quitActIcon;
    quitActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/quitIcon.png", "$Color-QuitActIcon")), QIcon::Normal);
    quitActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/quitIcon.png", "$Color-QuitActIcon-Active")), QIcon::Active);
    quitAct->setIcon(quitActIcon);

    // Edit menu
    QIcon selectObjectActIcon;
    selectObjectActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/select_object.png", "$Color-SelectObjectActIcon")), QIcon::Normal);
    selectObjectActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/select_object.png", "$Color-SelectObjectActIcon-Active")), QIcon::Active);
    selectObjectAct->setIcon(selectObjectActIcon);

    // View menu
    QIcon resetAllViewportsActIcon;
    resetAllViewportsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_refresh_black_48dp.png", "$Color-ResetAllViewportsActIcon")), QIcon::Normal);
    resetAllViewportsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_refresh_black_48dp.png", "$Color-ResetAllViewportsActIcon-Active")), QIcon::Active);
    resetAllViewportsAct->setIcon(resetAllViewportsActIcon);

    QIcon autoViewActIcon;
    autoViewActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_free_black_48dp.png", "$Color-AutoViewActIcon")), QIcon::Normal);
    autoViewActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_free_black_48dp.png", "$Color-AutoViewActIcon-Active")), QIcon::Active);
    autoViewAct->setIcon(autoViewActIcon);

    QIcon centerViewActIcon;
    centerViewActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_center_focus_strong_black_48dp.png", "$Color-CenterViewActIcon")), QIcon::Normal);
    centerViewActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_center_focus_strong_black_48dp.png", "$Color-CenterViewActIcon-Active")), QIcon::Active);
    centerViewAct->setIcon(centerViewActIcon);

    QIcon toggleGridActIcon;
    toggleGridActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_grid_on_black_48dp.png", "$Color-ToggleGridActIcon")), QIcon::Normal);
    toggleGridActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_grid_on_black_48dp.png", "$Color-ToggleGridActIcon-Active")), QIcon::Active);
    toggleGridAct->setIcon(toggleGridActIcon);

    // Raytrace menu
    QIcon raytraceActIcon;
    raytraceActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_filter_vintage_black_48dp.png", "$Color-RaytraceActIcon")), QIcon::Normal);
    raytraceActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_filter_vintage_black_48dp.png", "$Color-RaytraceActIcon-Active")), QIcon::Active);
    raytraceAct->setIcon(raytraceActIcon);
}

void MainWindow::prepareDockables(){
    // Object tree
    objectTreeWidgetDockable = new Dockable("Objects", this, false, 200);
    addDockWidget(Qt::LeftDockWidgetArea, objectTreeWidgetDockable);

    // Properties
    objectPropertiesDockable = new Dockable("Properties", this,true,300);
    addDockWidget(Qt::RightDockWidgetArea, objectPropertiesDockable);



    // Toolbox
//    toolboxDockable = new Dockable("Make", this,true,30);
//    toolboxDockable->hideHeader();
//    addDockWidget(Qt::LeftDockWidgetArea, toolboxDockable);
}

// empty new file
void MainWindow::newFile() {
    Document* document = new Document(documentsCount);
    document->getObjectTreeWidget()->setObjectName("dockableContent");
    document->getProperties()->setObjectName("dockableContent");
    documents[documentsCount++] = document;
    QString filename("Untitled");
    const int tabIndex = documentArea->addTab(document->getViewportGrid(), filename);
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
        const int tabIndex = documentArea->addTab(document->getViewportGrid(), filename);
        documentArea->setCurrentIndex(tabIndex);
        connect(documents[activeDocumentId]->getObjectTreeWidget(), &ObjectTreeWidget::selectionChanged,
                this, &MainWindow::objectTreeWidgetSelectionChanged);
    }
}

bool MainWindow::saveFile(const QString& filePath) {
    if (!documents[activeDocumentId]->isModified()) {
        return false;
    }

    return documents[activeDocumentId]->Save(filePath.toUtf8().data());
}

bool MainWindow::saveFileId(const QString& filePath, int documentId) {
    return documents[documentId]->Save(filePath.toUtf8().data());
}

void MainWindow::openFileDialog() 
{
	const QString filePath = QFileDialog::getOpenFileName(documentArea, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()){
        openFile(filePath);
    }
}

void MainWindow::saveAsFileDialog() {
    if (activeDocumentId == -1) return;
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

bool MainWindow::saveAsFileDialogId(int documentId) {
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()) {
        if (saveFileId(filePath, documentId))
        {
            documents[documentId]->setFilePath(filePath);
            QString filename(QFileInfo(filePath).fileName());
            documentArea->setTabText(documentArea->currentIndex(), filename);
            statusBarPathLabel->setText(*documents[documentId]->getFilePath());
            statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
            return true;
        }
    }

    return false;
}

void MainWindow::saveFileDefaultPath() {
    if (activeDocumentId == -1) return;
    if (documents[activeDocumentId]->getFilePath() == nullptr) saveAsFileDialog();
    else {
        const QString filePath = *documents[activeDocumentId]->getFilePath();
        if (!filePath.isEmpty()) {
            if (saveFile(filePath)) {
                statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
            }
        }
    }
}

bool MainWindow::saveFileDefaultPathId(int documentId) {
    if (documentId == -1) return false;
    if (documents[documentId]->getFilePath() == nullptr) return saveAsFileDialogId(documentId);
    
    const QString filePath = *documents[documentId]->getFilePath();
    if (!filePath.isEmpty()) {
        if (saveFileId(filePath, documentId)) {
            statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
            return true;
        }
    }

    return false;
}

bool MainWindow::maybeSave(int documentId, bool *cancel) {
    // Checks if the document has any unsaved changes
    if (documents[documentId]->isModified()) {
        QFileInfo pathName(documents[documentId]->getFilePath() != nullptr ? *documents[documentId]->getFilePath() : "Untitled");

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Do you want to save the changes you made to " + pathName.fileName() + "?");
        msgBox.setInformativeText("Your changes will be lost if you don't save them.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        msgBox.setDefaultButton(QMessageBox::Save);

        if (cancel != nullptr) {
            msgBox.addButton(QMessageBox::Cancel);
            *cancel = false;
        }
            
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Save:
            if (saveFileDefaultPathId(documentId)) {
                return true;
            }

            return false;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
            assert(cancel != nullptr);
            *cancel = true;
            return false;
        }
    }

    return true;
}

void MainWindow::onActiveDocumentChanged(const int newIndex){
    ViewportGrid * displayGrid = dynamic_cast<ViewportGrid*>(documentArea->widget(newIndex));
    if (displayGrid != nullptr){
        if (displayGrid->getDocument()->getDocumentId() != activeDocumentId){
            activeDocumentId = displayGrid->getDocument()->getDocumentId();
            objectTreeWidgetDockable->setContent(documents[activeDocumentId]->getObjectTreeWidget());
            objectPropertiesDockable->setContent(documents[activeDocumentId]->getProperties());
            statusBarPathLabel->setText(documents[activeDocumentId]->getFilePath()  != nullptr ? *documents[activeDocumentId]->getFilePath() : "Untitled");

            if(documents[activeDocumentId]->getViewportGrid()->inQuadViewportMode()){
                currentViewport->setCurrentIndex(4);
                for(QAction * action:singleViewAct) action->setChecked(false);
            }else {
                currentViewport->setCurrentIndex(documents[activeDocumentId]->getViewportGrid()->getActiveViewportId());
                for(QAction * action:singleViewAct) action->setChecked(false);
                singleViewAct[documents[activeDocumentId]->getViewportGrid()->getActiveViewportId()]->setChecked(true);
            }
        }
    }else if (activeDocumentId != -1){
        objectTreeWidgetDockable->clear();
        objectPropertiesDockable->clear();
        statusBarPathLabel->setText("");
        activeDocumentId = -1;
    }
}

void MainWindow::tabCloseRequested(const int i)
{
    int documentId = -1;
    ViewportGrid* displayGrid = dynamic_cast<ViewportGrid*>(documentArea->widget(i));
    
    if (displayGrid != nullptr) {
        documentId = displayGrid->getDocument()->getDocumentId();

        if (!maybeSave(documentId)) {
            return;
        }
    }
    documentArea->removeTab(i);
    if (documentId != -1) {
        delete documents[documentId];
        documents.erase(documentId);
    }
    if (documentArea->currentIndex() == -1){
        objectTreeWidgetDockable->clear();
        objectPropertiesDockable->clear();
        statusBarPathLabel->setText("");
        activeDocumentId = -1;
    }
}

void MainWindow::objectTreeWidgetSelectionChanged(int objectId) {
    documents[activeDocumentId]->getProperties()->bindObject(objectId);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    int documentSize = documents.size();
    bool cancel = false;
    ViewportGrid * displayGrid = nullptr;

    for (int documentIndex = 1; documentIndex <= documentSize; ++documentIndex) {
        displayGrid = dynamic_cast<ViewportGrid*>(documentArea->widget(documentIndex));
        int documentId = displayGrid->getDocument()->getDocumentId();
        
        if (maybeSave(documentId, &cancel)) {
            continue;
        }
        else {
            break;
        }
    }

    if (cancel == true) {
        event->ignore();
    }
    else {
        event->accept();
    }
}

void MainWindow::moveCameraButtonAction() {
    if (activeDocumentId != -1) {
        ViewportGrid* displayGrid = documents[activeDocumentId]->getViewportGrid();

        if (displayGrid != nullptr) {
            displayGrid->setMoveCameraMouseAction();
        }
    }
}

void MainWindow::selectObjectButtonAction() {
    ViewportGrid* displayGrid = documents[activeDocumentId]->getViewportGrid();

    if (displayGrid != nullptr) {
        displayGrid->setSelectObjectMouseAction();
    }
}

void MainWindow::updateMouseButtonObjectState() {
    if (activeDocumentId == -1) {
        selectObjectAct->setChecked(false);
        return;
    }

    if (selectObjectAct->isChecked() == true) {
        documents[activeDocumentId]->getViewportGrid()->getActiveViewport()->moveCameraEnabled = false;
        selectObjectAct->setToolTip("Select Object OFF");
        selectObjectButtonAction();
    }
    else if (documents[activeDocumentId]->getViewportGrid()->getActiveViewport()->moveCameraEnabled == false) {
        documents[activeDocumentId]->getViewportGrid()->getActiveViewport()->moveCameraEnabled = true;
        selectObjectAct->setToolTip("Select Object ON");
        moveCameraButtonAction();
    }
}
