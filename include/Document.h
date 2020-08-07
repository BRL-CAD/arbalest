//
// Created by Sadeep on 11-Jun.
//

#ifndef RT3_DOCUMENT_H
#define RT3_DOCUMENT_H

#include "Display.h"
#include "ObjectTree.h"
#include "ObjectTreeWidget.h"
#include "Properties.h"
#include "GeometryRenderer.h"

class Properties;
class Display;
class GeometryRenderer;

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

    void modifyObject(BRLCAD::Object* newObject);

    // getters setters
    QString* getFilePath() const
    {
	    return filePath;
    }

    BRLCAD::MemoryDatabase* getDatabase() const
    {
	    return database;
    }

    Display* getDisplay() const
    {
	    return display;
    }

    ObjectTreeWidget* getObjectTreeWidget() const
    {
	    return objectTreeWidget;
    }

    Properties* getProperties() const
    {
	    return properties;
    }

    int getDocumentId() const
    {
	    return documentId;
    }

    ObjectTree* getObjectTree() const
    {
	    return objectTree;
    }

    void setFilePath(const QString& filePath)
    {
        this->filePath = new QString(filePath);
    }

};


#endif //RT3_DOCUMENT_H
