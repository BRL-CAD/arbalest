//
// Created by Sadeep on 11-Jun.
//

#ifndef RT3_DOCUMENT_H
#define RT3_DOCUMENT_H


#include "GeometryOperationsManager.h"
#include "Display.h"

class Document {
private:
    BRLCAD::MemoryDatabase *database;
    GeometryOperationsManager *geometryOperationsManager;
    Display *display;
public:

    GeometryOperationsManager *getGeometryOperationsManager() const;
    Display *getDisplay() const;
    explicit Document(const char *filePath);
    std::vector<std::string> getObjectsList();
    void buildVectorList();
};


#endif //RT3_DOCUMENT_H
