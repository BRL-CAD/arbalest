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
#include "VerificationValidationWidget.h"
#include <include/RaytraceView.h>

class Properties;
class Display;
class GeometryRenderer;
class DisplayGrid;
class RaytraceView;
class ObjectTreeWidget;
class MainWindow;

class Document {
private:
    MainWindow* mainWindow;
    QString *filePath = nullptr;
    BRLCAD::MemoryDatabase *database;
    DisplayGrid *displayGrid;
    ObjectTreeWidget *objectTreeWidget;
    VerificationValidationWidget* vvWidget;
    Properties *properties;
    const int documentId;
    ObjectTree* objectTree;
    GeometryRenderer * geometryRenderer;
    bool modified;


public:
    explicit Document(MainWindow* mainWindow, int documentId, const QString *filePath = nullptr);
    virtual ~Document();

    void modifyObject(BRLCAD::Object* newObject);

    RaytraceView * raytraceWidget;
    // getters setters
    int getTabIndex();

    QString* getFilePath() const
    {
	    return filePath;
    }

    RaytraceView * getRaytraceWidget() const
    {
        return raytraceWidget;
    }
    BRLCAD::ConstDatabase* getDatabase() const
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

    VerificationValidationWidget* getVerificationValidationWidget() const {
        return vvWidget;
    }

    void loadVerificationValidationWidget();

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

    bool isModified();
    bool Add(const BRLCAD::Object& object);
    bool Save(const char* fileName);
    void getBRLCADConstObject(const QString& objectName, const std::function<void(const BRLCAD::Object&)>& func);
    void getBRLCADObject(const QString& objectName, const std::function<void(BRLCAD::Object&)>& func);
};


#endif //RT3_DOCUMENT_H
