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
#include "DisplayGrid.h"
#include <include/RaytraceView.h>

class Properties;
class Display;
class GeometryRenderer;
class DisplayGrid;
class RaytraceView;
class ObjectTreeWidget;

class Document {
private:
    QString *filePath = nullptr;
    BRLCAD::MemoryDatabase *database;
    DisplayGrid *displayGrid;
    ObjectTreeWidget *objectTreeWidget;
    Properties *properties;
    const int documentId;
    ObjectTree* objectTree;
    GeometryRenderer * geometryRenderer;


public:
    explicit Document(int documentId, const QString *filePath = nullptr);
    virtual ~Document();

    void modifyObject(BRLCAD::Object* newObject);

    RaytraceView * raytraceWidget;
    // getters setters
    QString* getFilePath() const
    {
	    return filePath;
    }

    RaytraceView * getRaytraceWidget() const
    {
        return raytraceWidget;
    }
    BRLCAD::MemoryDatabase* getDatabase() const
    {
        return database;
    }

    Display* getDisplay();

    ObjectTreeWidget* getObjectTreeWidget() const
    {
	    return objectTreeWidget;
    }

    Properties* getProperties() const
    {
	    return properties;
    }

    DisplayGrid *getDisplayGrid()  {
        return displayGrid;
    }

    int getDocumentId() const
    {
	    return documentId;
    }

    ObjectTree* getObjectTree() const
    {
	    return objectTree;
    }
    GeometryRenderer *getGeometryRenderer(){
        return geometryRenderer;
    }

    void setFilePath(const QString& filePath)
    {
        this->filePath = new QString(filePath);
    }

    void modifyObjectNoSet(BRLCAD::Object *newObject);
};


#endif //RT3_DOCUMENT_H
