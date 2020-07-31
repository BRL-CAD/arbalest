//
// Created by Sadeep on 11-Jun.
//

#ifndef RT3_DOCUMENT_H
#define RT3_DOCUMENT_H

#include "Display.h"
#include "ObjectTree.h"
#include "ObjectTreeWidget.h"
#include "Properties.h"

class Properties;
class Display;

class Document {
private:
    QString *filePath = nullptr;
    BRLCAD::MemoryDatabase *database;
    Display *display;
    ObjectTreeWidget *objectTreeWidget;
    Properties *properties;
    const int documentId;
    ObjectTree* objectTree;

public:
    explicit Document(int documentId, const QString *filePath = nullptr);
    virtual ~Document();
    void onDatabaseUpdated() const;

    // getters setters
    BRLCAD::MemoryDatabase *getDatabase() const;
    Display *getDisplay() const;
    ObjectTreeWidget *getObjectTreeWidget() const;
    Properties *getProperties() const;
    int getDocumentId() const;
    const QString *getFilePath() const;
    void setFilePath(const QString& filePath);
};


#endif //RT3_DOCUMENT_H
