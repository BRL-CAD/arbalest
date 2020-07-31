//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <iostream>
#include <ObjectTreeWidget.h>


Document::Document(const int documentId, const QString *filePath) : documentId(documentId) {
    if (filePath != nullptr) this->filePath = new QString(*filePath);
    database =  new BRLCAD::MemoryDatabase();
    if (filePath != nullptr) {
        if (!database->Load(filePath->toUtf8().data()))
        {
            throw -1;
        }
    }

    display = new Display(documentId);
    objectTree = new ObjectTree(database);
    objectTreeWidget = new ObjectTreeWidget(objectTree);
    properties = new Properties(*this);

    display->onDatabaseOpen(database);
    display->refresh();
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

Display *Document::getDisplay() const {
    return display;
}

ObjectTreeWidget *Document::getObjectTreeWidget() const {
    return objectTreeWidget;
}


void Document::onDatabaseUpdated() {
    display->onDatabaseUpdated();
}

const QString *Document::getFilePath() const {
    return filePath;
}

void Document::setFilePath(const QString& filePath)
{
    this->filePath = new QString(filePath);
}

Properties *Document::getProperties() const {
    return properties;
}

BRLCAD::MemoryDatabase *Document::getDatabase() {
    return database;
}
