//
// Created by Sadeep on 11-Jun.
//

#ifndef RT3_DOCUMENT_H
#define RT3_DOCUMENT_H


#include <QtWidgets/QMdiSubWindow>
#include "GeometryOperationsManager.h"
#include "Display.h"
#include "ObjectTree.h"
#include "Properties.h"

class Properties;
class Document {
private:
    BRLCAD::MemoryDatabase *database;
    GeometryOperationsManager *geometryOperationsManager;
    Display *display;
    ObjectTree *objectTree;
    Properties *properties;
    const int documentId;
    const QString filePath;

public:
    explicit Document(const char *filePath, int documentId);
    virtual ~Document();
    void onDatabaseUpdated();

    // getters
    GeometryOperationsManager *getGeometryOperationsManager() const;
    MemoryDatabase *getDatabase();
    Display *getDisplay() const;
    ObjectTree *getObjectTree() const;
    Properties *getProperties() const;

    std::vector<std::string> getTopObjectsList();
    const QString &getFilePath() const;
};


#endif //RT3_DOCUMENT_H
