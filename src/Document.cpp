//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <iostream>
#include <ObjectTree.h>


Document::Document(const char *filePath, int documentId) : filePath(QString(filePath)),documentId(documentId) {
    database =  new BRLCAD::MemoryDatabase();
    database->Load(filePath);

    display = new Display(documentId);
    geometryOperationsManager = new GeometryOperationsManager(*database);
    objectTree = new ObjectTree(*database);
    properties = new Properties(*this);

    display->onDatabaseOpen(database);
    display->refresh();
    objectTree->Rebuild();
}


Document::~Document() {
    delete database;
    delete display;
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

ObjectTree *Document::getObjectTree() const {
    return objectTree;
}


void Document::onDatabaseUpdated() {
    display->onDatabaseUpdated();
}

const QString &Document::getFilePath() const {
    return filePath;
}

Properties *Document::getProperties() const {
    return properties;
}

MemoryDatabase *Document::getDatabase() {
    return database;
}
