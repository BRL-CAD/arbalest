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
            throw std::exception("Failed to open file");
        }
    }

    objectTree = new ObjectTree(database);
    objectTreeWidget = new ObjectTreeWidget(objectTree);
    properties = new Properties(*this);
    objectTree->changeSubTreeVisibility(objectTree->getRootObjectId(), true);

    display = new Display(this);
    display->refresh();
}

Document::~Document() {
    delete database;
    delete display;
}
