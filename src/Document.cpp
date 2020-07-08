//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <iostream>
#include <ObjectsTreeView.h>


Document::Document(const char *filePath) {
    database =  new BRLCAD::MemoryDatabase();
    database->Load(filePath);

    display = new Display();
    geometryOperationsManager = new GeometryOperationsManager(*database);
    objectsTree = new ObjectsTreeView(*database);
    window = new QMdiSubWindow;

    window->setWidget(display);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setMinimumSize(windowMinimumWidth, windowMinimumHeight);
    window->setWindowTitle(filePath);

    display->onDatabaseOpen(database);
    display->refresh();
    objectsTree->Rebuild();
}


Document::~Document() {
    delete database;
    delete display;
    delete window;
}



std::vector<std::string> Document::getTopObjectsList() {
    std::vector<std::string> list;

    BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();
    while (it.Good()){
        list.push_back(it.Name());
        ++it;
    }
    return list;
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

void Document::onDatabaseUpdated() {
    display->onDatabaseUpdated();
}