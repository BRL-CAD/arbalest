//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <ArbalestGlobals.h>
//#include <VectorListRenderer.h>
#include "rt/vlist.h"
#include <brlcad/cicommon.h>
#include <brlcad/Object.h>
#include <iostream>
#include <brlcad/bn/vlist.h>

Document::Document(const char *filePath) {
    database =  new BRLCAD::MemoryDatabase();
    database->Load(filePath);

    display = new Display();
    geometryOperationsManager = new GeometryOperationsManager(database);

    ArbalestGlobals::documents.push_back(this);

    buildVectorList();

    display->refresh();
}


std::vector<std::string> Document::getObjectsList() {
    std::vector<std::string> list;

    BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();
    while (it.Good()){
        list.push_back(it.Name());
        ++it;
    }
    return list;
}

void Document::buildVectorList(){
    display->getVectorLists().clear();
    for (const auto& objectName: getObjectsList()) {
        VectorList * vl = new VectorList();
        database->Plot(objectName.c_str(), *vl);
        display->getVectorLists().push_back(vl);


    }

}

GeometryOperationsManager *Document::getGeometryOperationsManager() const {
    return geometryOperationsManager;
}

Display *Document::getDisplay() const {
    return display;
}
