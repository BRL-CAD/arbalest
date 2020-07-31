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

    display = new Display(this);
    display->refresh();
}

Document::~Document() {
    delete database;
    delete display;
}

Display *Document::getDisplay() const {
    return display;
}

ObjectTreeWidget *Document::getObjectTreeWidget() const {
    return objectTreeWidget;
}

void Document::onDatabaseUpdated() const
{
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

int Document::getDocumentId() const
{
	return documentId;
}

BRLCAD::MemoryDatabase *Document::getDatabase() const
{
    return database;
}
