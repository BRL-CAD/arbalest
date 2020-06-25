//
// Created by Sadeep on 11-Jun.
//

#ifndef RT3_DOCUMENT_H
#define RT3_DOCUMENT_H


#include <QtOpenGL/QtOpenGL>
#include "GeometryOperationsManager.h"
#include "Display.h"
#include "ObjectsTreeView.h"

class Document {
private:
    BRLCAD::MemoryDatabase *database;
    GeometryOperationsManager *geometryOperationsManager;
    Display *display;
    ObjectsTreeView *objectsTree;
    QMdiSubWindow * window;
    const int windowMinimumHeight = 500;
    const int windowMinimumWidth = 700;
public:

    explicit Document(const char *filePath);

    // getters
    void refreshGeometry();
    GeometryOperationsManager *getGeometryOperationsManager() const;
    Display *getDisplay() const;
    ObjectsTreeView *getObjectsTree() const;

    QMdiSubWindow *getWindow() const;

    std::vector<std::string> getObjectsList();
    void buildVectorListInDisplay();
};


#endif //RT3_DOCUMENT_H
