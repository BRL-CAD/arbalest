#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include "DisplayGrid.h"
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
#include <brlcad/Arb8.h>
#include <QtWidgets/QtWidgets>
#include <brlcad/Torus.h>
#include <brlcad/Cone.h>
#include <brlcad/Particle.h>
#include <brlcad/Paraboloid.h>
#include <brlcad/Hyperboloid.h>
#include <brlcad/Halfspace.h>
#include <brlcad/EllipticalTorus.h>
#include <brlcad/Ellipsoid.h>
#include <brlcad/HyperbolicCylinder.h>
#include <brlcad/ParabolicCylinder.h>
#include <include/MatrixTransformWidget.h>


using namespace BRLCAD;
using namespace std;



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    loadTheme();
    prepareUi();
    prepareDockables();

    documentArea->addTab(new HelpWidget(), "Quick Start");
    if(QCoreApplication::arguments().length()>1){
        QString filePath = QString(QCoreApplication::arguments().at(1));
        if (filePath.endsWith(".atr")) openATRFile(filePath);
        else openFile(filePath);
    }
    Globals::mainWindow = this;
}

MainWindow::~MainWindow()
{
    for (const std::pair<const int, Document*>& pair : documents) {
        Document* document = pair.second;
        delete document;
    }
}

void MainWindow::loadTheme()
{
    QSettings settings("BRLCAD", "arbalest");
    int themeIndex = settings.value("themeIndex",0).toInt();

    QStringList themes = {":themes/arbalest_light.theme",":themes/arbalest_dark.theme"};

	QFile themeFile(themes[themeIndex] );

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
    setWindowTitle("Arbalest");
	// Menu bar -------------------------------------------------------------------------------------------------------------
    menuTitleBar = new QMenuBar(this);
    menuTitleBar->installEventFilter(this);
    setMenuBar(menuTitleBar);

    QMenu *fileMenu = menuTitleBar->addMenu(tr("&File"));

    QIcon newActIcon;
    newActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-MenuIconFile")), QIcon::Normal);
    newActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-Menu")), QIcon::Active);
    QAction* newAct = new QAction(newActIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("New .g file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);

    QIcon openActIcon;
    openActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-MenuIconFile")), QIcon::Normal);
    openActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-Menu")), QIcon::Active);
    QAction* openAct = new QAction(openActIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Opens a .g file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFileDialog);
    fileMenu->addAction(openAct);

    QIcon saveActIcon;
    saveActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-MenuIconFile")), QIcon::Normal);
    saveActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-Menu")), QIcon::Active);
    QAction* saveAct = new QAction(saveActIcon, tr("Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save database"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFileDefaultPath);
    fileMenu->addAction(saveAct);

    QIcon saveAsActIcon;
    saveAsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-MenuIconFile")), QIcon::Normal);
    saveAsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-Menu")), QIcon::Active);
    QAction* saveAsAct = new QAction(saveAsActIcon, tr("Save As..."), this);
    saveAsAct->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    saveAsAct->setStatusTip(tr("Save database as"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFileDialog);
    fileMenu->addAction(saveAsAct);
    
    fileMenu->addSeparator();

    QIcon quitActIcon;
    quitActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/quitIcon.png", "$Color-MenuIconQuit")), QIcon::Normal);
    QAction* quitAct = new QAction(quitActIcon, tr("Quit"), this);
    quitAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
    quitAct->setStatusTip(tr("Quit"));
    connect(quitAct, &QAction::triggered, this,[this](){
        QCoreApplication::quit();
    });
    fileMenu->addAction(quitAct);

    QMenu* createMenu = menuTitleBar->addMenu(tr("&Create"));

    QAction* createArb8Act = new QAction(tr("Arb8"), this);
    connect(createArb8Act, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Arb8 * object = new BRLCAD::Arb8();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createArb8Act);

    QAction* createConeAct = new QAction(tr("Cone"), this);
    connect(createConeAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Cone * object = new BRLCAD::Cone();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createConeAct);




    QAction* createEllipsoidAct = new QAction(tr("Ellipsoid"), this);
    connect(createEllipsoidAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Ellipsoid * object = new BRLCAD::Ellipsoid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createEllipsoidAct);


    QAction* createEllipticalTorusAct = new QAction(tr("Elliptical Torus"), this);
    connect(createEllipticalTorusAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::EllipticalTorus * object = new BRLCAD::EllipticalTorus();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createEllipticalTorusAct);



    QAction* createHalfspaceAct = new QAction(tr("Halfspace"), this);
    connect(createHalfspaceAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Halfspace * object = new BRLCAD::Halfspace();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createHalfspaceAct);



    QAction* createHyperbolicCylinderAct = new QAction(tr("Hyperbolic Cylinder"), this);
    connect(createHyperbolicCylinderAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::HyperbolicCylinder * object = new BRLCAD::HyperbolicCylinder();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createHyperbolicCylinderAct);



    QAction* createHyperboloidAct = new QAction(tr("Hyperboloid"), this);
    connect(createHyperboloidAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Hyperboloid * object = new BRLCAD::Hyperboloid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createHyperboloidAct);




    QAction* createParabolicCylinderAct = new QAction(tr("Parabolic Cylinder"), this);
    connect(createParabolicCylinderAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::ParabolicCylinder * object = new BRLCAD::ParabolicCylinder();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createParabolicCylinderAct);




    QAction* createParaboloidAct = new QAction(tr("Paraboloid"), this);
    connect(createParaboloidAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Paraboloid * object = new BRLCAD::Paraboloid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createParaboloidAct);



    QAction* createParticleAct = new QAction(tr("Particle"), this);
    connect(createParticleAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Particle * object = new BRLCAD::Particle();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createParticleAct);


    QAction* createTorusAct = new QAction(tr("Torus"), this);
    connect(createTorusAct, &QAction::triggered, this,[this](){
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Torus * object = new BRLCAD::Torus();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getDisplayGrid()->forceRerenderAllDisplays();
    });
    createMenu->addAction(createTorusAct);

    QMenu* editMenu = menuTitleBar->addMenu(tr("&Edit"));

    QAction* relativeMoveAct = new QAction("Relative move selected object", this);
    relativeMoveAct->setStatusTip(tr("Relative move selected object. Top objects cannot be moved."));
    connect(relativeMoveAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Translate);
    });
    editMenu->addAction(relativeMoveAct);

    QAction* relativeScaleAct = new QAction("Relative scale selected object", this);
    relativeScaleAct->setStatusTip(tr("Relative scale selected object. Top objects cannot be scaled."));
    connect(relativeScaleAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Scale);
    });
    editMenu->addAction(relativeScaleAct);

    QAction* relativeRotateAct = new QAction("Relative rotate selected object", this);
    relativeRotateAct->setStatusTip(tr("Relative rotate selected object. Top objects cannot be rotated."));
    connect(relativeRotateAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Rotate);
    });
    editMenu->addAction(relativeRotateAct);


    QMenu* viewMenu = menuTitleBar->addMenu(tr("&View"));

    QAction* resetViewportAct = new QAction("Reset current viewport", this);
    resetViewportAct->setStatusTip(tr("Reset to default camera orientation for the viewport and autoview to currently visible objects"));
    connect(resetViewportAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getDisplayGrid()->resetViewPort(documents[activeDocumentId]->getDisplayGrid()->getActiveDisplayId());
    });
    viewMenu->addAction(resetViewportAct);

    QAction* resetAllViewportsAct = new QAction("Reset all viewports", this);
    resetAllViewportsAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_refresh_black_48dp.png", "$Color-MenuIconView")));
    resetAllViewportsAct->setStatusTip(tr("Reset to default camera orientation for each viewport and autoview to visible objects"));
    connect(resetAllViewportsAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getDisplayGrid()->resetAllViewPorts();
    });
    viewMenu->addAction(resetAllViewportsAct);

    viewMenu->addSeparator();

    QAction* autoViewAct = new QAction(tr("Focus visible objects (all viewports)"), this);
    autoViewAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_free_black_48dp.png", "$Color-MenuIconView")));
    autoViewAct->setShortcut(Qt::Key_F|Qt::CTRL);
    autoViewAct->setStatusTip(tr("Resize and center the view based on the current visible objects"));
    connect(autoViewAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        for(Display * display : documents[activeDocumentId]->getDisplayGrid()->getDisplays()){
            display->getCamera()->autoview();
            display->forceRerenderFrame();
        }
    });
    viewMenu->addAction(autoViewAct);

    QAction* autoViewSingleAct = new QAction(tr("Focus visible objects (current viewport)"), this);
    autoViewSingleAct->setStatusTip(tr("Resize and center the view based on the current visible objects"));
    connect(autoViewSingleAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getDisplay()->getCamera()->autoview();
        documents[activeDocumentId]->getDisplay()->forceRerenderFrame();
    });
    viewMenu->addAction(autoViewSingleAct);

    QAction* centerViewAct = new QAction(tr("Focus selected object"), this);
    centerViewAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_center_focus_strong_black_48dp.png", "$Color-MenuIconView")));
    centerViewAct->setStatusTip(tr("Resize and center the view based on the selected objects"));
    centerViewAct->setShortcut(Qt::Key_F);
    connect(centerViewAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        documents[activeDocumentId]->getDisplay()->getCamera()->centerView(objectId);
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
        singleViewAct[i]->setStatusTip("Display viewport " + QString::number(i+1));
        connect(singleViewAct[i], &QAction::triggered, this, [this,i]() {
            if (activeDocumentId == -1) return;
            documents[activeDocumentId]->getDisplayGrid()->singleDisplayMode(i);
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
    quadViewAct->setStatusTip(tr("Display 4 viewports"));
    connect(quadViewAct, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getDisplayGrid()->quadDisplayMode();
        for(QAction *i : singleViewAct) i->setChecked(false);
        currentViewport->setCurrentIndex(4);
    });
    quadViewAct->setShortcut(Qt::Key_5);
    viewMenu->addAction(quadViewAct);

    viewMenu->addSeparator();
    
    QAction* toggleGridAct = new QAction(tr("Toggle grid on/off"), this);
    toggleGridAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sharp_grid_on_black_48dp.png", "$Color-MenuIconView")));
    toggleGridAct->setCheckable(true);
    toggleGridAct->setShortcut(Qt::Key_G);
    connect(toggleGridAct, &QAction::toggled, this, [=]() {
        if (activeDocumentId == -1) {
            toggleGridAct->setChecked(false);
            return;
        }

        if (documents[activeDocumentId]->getDisplayGrid()->getActiveDisplay()->gridEnabled == false) {
            documents[activeDocumentId]->getDisplayGrid()->getActiveDisplay()->gridEnabled = true;
            toggleGridAct->setToolTip("Toggle grid OFF (G)");
        }
        else {
            documents[activeDocumentId]->getDisplayGrid()->getActiveDisplay()->gridEnabled = false;
            toggleGridAct->setToolTip("Toggle grid ON (G)");
        }

        documents[activeDocumentId]->getDisplayGrid()->getActiveDisplay()->forceRerenderFrame();
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


    QMenu* raytrace = menuTitleBar->addMenu(tr("&Raytrace"));
    QAction* raytraceAct = new QAction(tr("Raytrace current viewport"), this);
    raytraceAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_filter_vintage_black_48dp.png", "$Color-MenuIconRaytrace")));
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

    QMenu* verifyValidateMenu = menuTitleBar->addMenu(tr("&Verify/Validate"));
    verifyValidateViewportAct = new QAction(tr("Verify and validate current viewport"), this);
    verifyValidateViewportAct->setStatusTip(tr("Verify and validate current viewport"));
    verifyValidateViewportAct->setShortcut(Qt::CTRL|Qt::Key_B);
    updateVerifyValidateAct(documents[activeDocumentId]);
    verifyValidateMenu->addAction(verifyValidateViewportAct);

    QAction* verificationValidationNewTest = new QAction(tr("Create new test"), this);
    verificationValidationNewTest->setIcon(QPixmap::fromImage(coloredIcon(":/icons/verifyValidateCreateIcon.png", "$Color-MenuIconVerifyValidate")));
    verificationValidationNewTest->setStatusTip(tr("Create new test"));
    connect(verificationValidationNewTest, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getVerificationValidationWidget()->showNewTestDialog();
    });
    verifyValidateMenu->addAction(verificationValidationNewTest);

    QAction* verificationValidationRemoveTest = new QAction(tr("Remove test"), this);
    verificationValidationRemoveTest->setIcon(QPixmap::fromImage(coloredIcon(":/icons/verifyValidateRemoveIcon.png", "$Color-MenuIconVerifyValidate")));
    verificationValidationRemoveTest->setStatusTip(tr("Remove test"));
    connect(verificationValidationRemoveTest, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getVerificationValidationWidget()->showRemoveTestDialog();
    });
    verifyValidateMenu->addAction(verificationValidationRemoveTest);

    QAction* verificationValidationNewTestSuite = new QAction(tr("Create new test suite"), this);
    verificationValidationNewTestSuite->setIcon(QPixmap::fromImage(coloredIcon(":/icons/verifyValidateCreateIcon.png", "$Color-MenuIconVerifyValidate")));
    verificationValidationNewTestSuite->setStatusTip(tr("Create new test suite"));
    connect(verificationValidationNewTestSuite, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getVerificationValidationWidget()->showNewTestSuiteDialog();
    });
    verifyValidateMenu->addAction(verificationValidationNewTestSuite);

    QAction* verificationValidationRemoveTestSuite = new QAction(tr("Remove test suite"), this);
    verificationValidationRemoveTestSuite->setIcon(QPixmap::fromImage(coloredIcon(":/icons/verifyValidateRemoveIcon.png", "$Color-MenuIconVerifyValidate")));
    verificationValidationRemoveTestSuite->setStatusTip(tr("Remove test suite"));
    connect(verificationValidationRemoveTestSuite, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getVerificationValidationWidget()->showRemoveTestSuiteDialog();
    });
    verifyValidateMenu->addAction(verificationValidationRemoveTestSuite);

    QAction* verificationValidationExportToCSV = new QAction(tr("Export to CSV"), this);
    verificationValidationExportToCSV->setIcon(QPixmap::fromImage(coloredIcon(":/icons/exportIcon.png", "$Color-MenuIconVerifyValidate")));
    verificationValidationExportToCSV->setStatusTip(tr("Export to CSV"));
    connect(verificationValidationExportToCSV, &QAction::triggered, this, [this](){
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getVerificationValidationWidget()->exportToCSV();
    });
    verifyValidateMenu->addAction(verificationValidationExportToCSV);

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
            documentArea->insertTab(0,new HelpWidget,"Quick Start");
        }
        documentArea->setCurrentIndex(0);
    });
    help->addAction(helpAct);

    // Title bar [widgets in the menu bar] ----------------------------------------------------------------------------------------
    QPushButton* applicationIcon = new QPushButton(menuTitleBar);
    applicationIcon->setIcon(QIcon(":/icons/arbalest_icon.png"));
    applicationIcon->setObjectName("topLeftAppIcon");
    menuTitleBar->setCornerWidget(applicationIcon, Qt::TopLeftCorner);
    setWindowIcon(*new QIcon(*new QBitmap(":/icons/arbalest_icon.png")));

    QHBoxLayout* layoutTopRightWidget = new QHBoxLayout;
    layoutTopRightWidget->setContentsMargins(0, 0, 0, 0);
    QWidget* topRightWidget = new QWidget;
    topRightWidget->setLayout(layoutTopRightWidget);
    menuTitleBar->setCornerWidget(topRightWidget, Qt::TopRightCorner);
    layoutTopRightWidget->setSpacing(0);

    QPushButton* minimizeButton = new QPushButton(topRightWidget);
    minimizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_minimize_white_36dp","$Color-WindowTitleButtons")));
    minimizeButton->setObjectName("minimizeButton");
    connect(minimizeButton, &QPushButton::clicked, this, &MainWindow::minimizeButtonPressed);
    layoutTopRightWidget->addWidget(minimizeButton);


    maximizeButton = new QPushButton(topRightWidget);
    if (this->windowState() == Qt::WindowMaximized) {
        maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sadeep_edited_baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
    }
    else {
        maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
    }
    maximizeButton->setObjectName("maximizeButton");
    connect(maximizeButton, &QPushButton::clicked, this, &MainWindow::maximizeButtonPressed);
    layoutTopRightWidget->addWidget(maximizeButton);

    QPushButton* closeButton = new QPushButton(topRightWidget);
    closeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sadeep_edited_baseline_close_white_36dp","$Color-WindowTitleButtons")));
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

    QToolButton* newButton = new QToolButton(menuTitleBar);
    newButton->setDefaultAction(newAct);
    newButton->setObjectName("toolbarButton");
    QIcon newButtonIcon;
    newButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-IconFile")), QIcon::Normal);
    newButton->setIcon(newButtonIcon);
    newButton->setToolTip("New (Ctrl+N)");
    mainTabBarCornerWidget->addWidget(newButton);

    QToolButton* openButton = new QToolButton(menuTitleBar);
    openButton->setDefaultAction(openAct);
    openButton->setObjectName("toolbarButton");
    QIcon openButtonIcon;
    openButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-IconFile")), QIcon::Normal);
    openButton->setIcon(openButtonIcon);
    openButton->setToolTip("Open (Ctrl+O)");
    mainTabBarCornerWidget->addWidget(openButton);

    QToolButton* saveButton = new QToolButton(menuTitleBar);
    saveButton->setDefaultAction(saveAct);
    saveButton->setObjectName("toolbarButton");
    QIcon saveButtonIcon;
    saveButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-IconFile")), QIcon::Normal);
    saveButton->setIcon(saveButtonIcon);
    saveButton->setToolTip("Save (Ctrl+S)");
    mainTabBarCornerWidget->addWidget(saveButton);

    QToolButton* saveAsButton = new QToolButton(menuTitleBar);
    saveAsButton->setDefaultAction(saveAsAct);
    saveAsButton->setObjectName("toolbarButton");
    QIcon saveAsButtonIcon;
    saveAsButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-IconFile")), QIcon::Normal);
    saveAsButton->setIcon(saveAsButtonIcon);
    saveAsButton->setToolTip("Save as..");
    mainTabBarCornerWidget->addWidget(saveAsButton);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    QToolButton* focusAll = new QToolButton(menuTitleBar);
    focusAll->setDefaultAction(autoViewAct);
    focusAll->setObjectName("toolbarButton");
    QIcon focusAllIcon;
    focusAllIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_free_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    focusAll->setIcon(focusAllIcon);
    focusAll->setToolTip("Focus on all visible objects (Ctrl+F)");
    mainTabBarCornerWidget->addWidget(focusAll);

    QToolButton* focusCurrent = new QToolButton(menuTitleBar);
    focusCurrent->setDefaultAction(centerViewAct);
    focusCurrent->setObjectName("toolbarButton");
    QIcon focusCurrentIcon;
    focusCurrentIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_center_focus_strong_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    focusCurrent->setIcon(focusCurrentIcon);
    focusCurrent->setToolTip("Focus on selected object (F)");
    mainTabBarCornerWidget->addWidget(focusCurrent);

    QToolButton* resetViewports = new QToolButton(menuTitleBar);
    resetViewports->setDefaultAction(resetAllViewportsAct);
    resetViewports->setObjectName("toolbarButton");
     QIcon resetViewportsIcon;
    resetViewportsIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_refresh_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    resetViewports->setIcon(resetViewportsIcon);
    resetViewports->setToolTip("Reset the viewports and focus on the visible");
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
        if (index <4) documents[activeDocumentId]->getDisplayGrid()->singleDisplayMode(index);
        else documents[activeDocumentId]->getDisplayGrid()->quadDisplayMode();
        for(QAction *i : singleViewAct) i->setChecked(false);
        if(index != 4) singleViewAct[index]->setChecked(true);
    });
    mainTabBarCornerWidget->addWidget(currentViewport);

    QToolButton* toggleGrid = new QToolButton(menuTitleBar);
    toggleGrid->setDefaultAction(toggleGridAct);
    toggleGrid->setObjectName("toolbarButton");
    QIcon toggleGridIcon;
    toggleGridIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_grid_on_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    toggleGrid->setIcon(toggleGridIcon);
    mainTabBarCornerWidget->addWidget(toggleGrid);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    QToolButton* verifyValidate = new QToolButton(menuTitleBar);
    verifyValidate->setDefaultAction(verifyValidateViewportAct);
    verifyValidate->setObjectName("toolbarButton");
    QIcon verifyValidateIcon;
    verifyValidateIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/verifyValidateIcon.png", "$Color-IconView")), QIcon::Normal);
    verifyValidate->setIcon(verifyValidateIcon);
    verifyValidate->setToolTip("Verify and validate current viewport");
    mainTabBarCornerWidget->addWidget(verifyValidate);

    QToolButton* raytraceButton = new QToolButton(menuTitleBar);
    raytraceButton->setDefaultAction(raytraceAct);
    raytraceButton->setObjectName("toolbarButton");
    QIcon raytraceButtonIcon;
    raytraceButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_filter_vintage_black_48dp.png", "$Color-IconRaytrace")), QIcon::Normal);
    raytraceButton->setIcon(raytraceButtonIcon);
    raytraceButton->setToolTip("Raytrace current viewport (Ctrl+R)");
    mainTabBarCornerWidget->addWidget(raytraceButton);

    documentArea->setCornerWidget(mainTabBarCornerWidget,Qt::Corner::TopRightCorner);
}

void MainWindow::prepareDockables(){
    // Object tree
    objectTreeWidgetDockable = new Dockable("Objects", this, false, 200);
    addDockWidget(Qt::LeftDockWidgetArea, objectTreeWidgetDockable);

    // Properties
    objectPropertiesDockable = new Dockable("Properties", this,true,300);
    addDockWidget(Qt::RightDockWidgetArea, objectPropertiesDockable);

    objectVerificationValidationDockable = new Dockable("Verification & Validation", this, true, 300);
    addDockWidget(Qt::BottomDockWidgetArea, objectVerificationValidationDockable);
    objectVerificationValidationDockable->setVisible(false);

    // Toolbox
//    toolboxDockable = new Dockable("Make", this,true,30);
//    toolboxDockable->hideHeader();
//    addDockWidget(Qt::LeftDockWidgetArea, toolboxDockable);
}

// empty new file
void MainWindow::newFile() {
    Document* document = new Document(this, documentsCount);
    document->getObjectTreeWidget()->setObjectName("dockableContent");
    document->getProperties()->setObjectName("dockableContent");
    documents[documentsCount++] = document;
    QString filename("Untitled");
    const int tabIndex = documentArea->addTab(document->getDisplayGrid(), filename);
    documentArea->setCurrentIndex(tabIndex);
    connect(documents[activeDocumentId]->getObjectTreeWidget(), &ObjectTreeWidget::selectionChanged,
            this, &MainWindow::objectTreeWidgetSelectionChanged);
}

void MainWindow::openFile(const QString& filePath) {
    Document* document = nullptr;

    try {
        document = new Document(this, documentsCount, &filePath);
    }
    catch (const std::runtime_error& e) {
        QString msg = e.what();
        if (!msg.isEmpty()) popup(msg);
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
        const int tabIndex = documentArea->addTab(document->getDisplayGrid(), filename);
        documentArea->setCurrentIndex(tabIndex);
        connect(documents[activeDocumentId]->getObjectTreeWidget(), &ObjectTreeWidget::selectionChanged,
                this, &MainWindow::objectTreeWidgetSelectionChanged);
    }
}

void MainWindow::openATRFile(const QString& atrFilePath) {    
    {
        QString modelID = "", gFilePath = "";
        if (!QFile::exists(atrFilePath)) { popup("File " + atrFilePath + " doesn't exist."); return; }

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(atrFilePath);

        if (!db.open() || !db.isOpen()) { popup("Failed to open " + atrFilePath); return; }

        QSqlQuery q("SELECT id, filePath from Model", db);
        if (!q.isActive() || !q.next())  { popup("Failed to fetch Model table from " + atrFilePath); return; }
        else {
            modelID = q.value(0).toString();
            gFilePath = q.value(1).toString();
        }

        if (!gFilePath.isEmpty()) {
            // if associated .g doesn't exist/was moved, ask user to find it and update
            QMessageBox msgBox;
            QString newGFilePath;
            while (!QFile::exists(gFilePath)) { 
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("The file associated with these test results no longer exists at " + gFilePath + ".\nPlease update the file location.");
                msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Open);

                if (msgBox.exec() == QMessageBox::Open) {
                    newGFilePath = QFileDialog::getOpenFileName(documentArea, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
                    if (!newGFilePath.isEmpty()) gFilePath = newGFilePath;
                }
                else break;
            }

            if (QFile::exists(gFilePath)) {
                QString* newUUID = generateUUID(gFilePath);
                if (!newUUID) { popup("Failed to generate UUID for " + gFilePath); return; }
                q.prepare("UPDATE Model SET filepath = ?, uuid = ? WHERE id = ?");
                q.addBindValue(gFilePath);
                q.addBindValue(*newUUID);
                q.addBindValue(modelID);
                q.exec();
                if (!q.isActive()) { popup("Failed to update filepath to " + gFilePath + ".\n" + q.lastError().text()); return; }
                openFile(gFilePath);
            }
        }

        if (db.isOpen()) db.close();
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
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
	const QString filePath = QFileDialog::getOpenFileName(documentArea, 
    tr("Open BRL-CAD database"),
    QString(), 
    "BRL-CAD Database (*.g);; Arbalest Test Results (*.atr)");
    if (!filePath.isEmpty()){
        if (filePath.endsWith(".atr")) openATRFile(filePath);
        else openFile(filePath);
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
        QFileInfo pathName = documents[documentId]->getFilePath() != nullptr ? *documents[documentId]->getFilePath() : "Untitled";

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
    DisplayGrid * displayGrid = dynamic_cast<DisplayGrid*>(documentArea->widget(newIndex));
    if (displayGrid != nullptr){
        if (displayGrid->getDocument()->getDocumentId() != activeDocumentId){
            activeDocumentId = displayGrid->getDocument()->getDocumentId();
            objectTreeWidgetDockable->setContent(documents[activeDocumentId]->getObjectTreeWidget());
            objectPropertiesDockable->setContent(documents[activeDocumentId]->getProperties());
            objectVerificationValidationDockable->setContent(documents[activeDocumentId]->getVerificationValidationWidget());
            updateVerifyValidateAct(documents[activeDocumentId]);
            if (documents[activeDocumentId]->getVerificationValidationWidget())
                documents[activeDocumentId]->getVerificationValidationWidget()->updateDockableHeader();
            else
                objectVerificationValidationDockable->setVisible(false);
            objectVerificationValidationDockable->setVisible((documents[activeDocumentId]->getVerificationValidationWidget()) ? true : false);
            statusBarPathLabel->setText(documents[activeDocumentId]->getFilePath()  != nullptr ? *documents[activeDocumentId]->getFilePath() : "Untitled");

            if(documents[activeDocumentId]->getDisplayGrid()->inQuadDisplayMode()){
                currentViewport->setCurrentIndex(4);
                for(QAction * action:singleViewAct) action->setChecked(false);
            }else {
                currentViewport->setCurrentIndex(documents[activeDocumentId]->getDisplayGrid()->getActiveDisplayId());
                for(QAction * action:singleViewAct) action->setChecked(false);
                singleViewAct[documents[activeDocumentId]->getDisplayGrid()->getActiveDisplayId()]->setChecked(true);
            }
        }
    }else if (activeDocumentId != -1){
        objectVerificationValidationDockable->setVisible(false);
        objectTreeWidgetDockable->clear();
        objectPropertiesDockable->clear();
        objectVerificationValidationDockable->clear();
        statusBarPathLabel->setText("");
        activeDocumentId = -1;
    }
}

void MainWindow::tabCloseRequested(const int i)
{
    int documentId = -1;
    DisplayGrid* displayGrid = dynamic_cast<DisplayGrid*>(documentArea->widget(i));
    
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
        objectVerificationValidationDockable->clear();
        statusBarPathLabel->setText("");
        activeDocumentId = -1;
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
    if(!isMaximized()) showMaximized();
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
            maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sadeep_edited_baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
        }
        else{
            maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    int documentSize = documents.size();
    bool cancel = false;
    DisplayGrid * displayGrid = nullptr;

    for (int documentIndex = 1; documentIndex <= documentSize; ++documentIndex) {
        displayGrid = dynamic_cast<DisplayGrid*>(documentArea->widget(documentIndex));
        if (!displayGrid) continue;
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