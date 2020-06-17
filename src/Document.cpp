//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <ArbalestSettings.h>
#include <VListRenderer.h>
#include "rt/vlist.h"
#include <brlcad/cicommon.h>
#include <brlcad/Object.h>
#include <iostream>
#include <ObjectsTreeView.h>
#include <QtOpenGL/QtOpenGL>

#include <gl/GL.h>
#include "VListRenderer.h"
#include "vmath.h"
#include <bn.h>
#include "raytrace.h"
#include <brlcad/ConstDatabase.h>
#include <brlcad/MemoryDatabase.h>


Document::Document(const char *filePath) {
    database =  new BRLCAD::MemoryDatabase();
    database->Load(filePath);

    display = new Display();
    geometryOperationsManager = new GeometryOperationsManager(database);
    objectsTree = new ObjectsTreeView(*database);
    window = new QMdiSubWindow;

    window->setWidget(display);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setMinimumSize(windowMinimumWidth, windowMinimumHeight);
    window->setWindowTitle(filePath);

    buildVectorListInDisplay();
    display->refresh();
    objectsTree->Rebuild();
}


std::vector<std::string> Document::getObjectsList() {
    std::vector<std::string> list;

    BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();
    while (it.Good()){
        list.push_back(it.Name());
        ++it;
    }
    return list;
}



void Document::buildVectorListInDisplay(){
    if (ArbalestSettings::useLegacyVlists){
        //display->setVlist(reinterpret_cast<bn_vlist *>(Plot(database, "Goliath.c")));
        display->getVectorLists().clear();
        for (const auto &objectName: getObjectsList()) {
            auto *vl = new VectorList();
            database->Plot(objectName.c_str(), *vl);
            display->getVectorLists().push_back(vl);
        }
    }
    else {
        display->getVectorLists().clear();
        for (const auto &objectName: getObjectsList()) {
            auto *vl = new VectorList();
            database->Plot(objectName.c_str(), *vl);
            display->getVectorLists().push_back(vl);
        }
    }
}

GeometryOperationsManager *Document::getGeometryOperationsManager() const {
    return geometryOperationsManager;
}

Display *Document::getDisplay() const {
    return display;
}

ObjectsTreeView *Document::getObjectsTree() const {
    return objectsTree;
}

QMdiSubWindow *Document::getWindow() const {
    return window;
}
