//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <ArbalestGlobals.h>

Document::Document(const char *filePath) {
    database =  new BRLCAD::MemoryDatabase();
    database->Load(filePath);

    display = new Display();
    geometryOperationsManager = new GeometryOperationsManager(database);

    ArbalestGlobals::documents.push_back(this);
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

std::vector<BRLCAD::VectorList::Element*> list;
std::map<std::string,std::vector<BRLCAD::VectorList::Element*>> Document::getVectorLists(){
    std::map<std::string,std::vector<BRLCAD::VectorList::Element*>> lists;

    for (const auto& objectName: getObjectsList()) {
        VectorList vl;
        database->Plot(objectName.c_str(), vl);

        class Callback : public BRLCAD::VectorList::ElementCallback {
            bool operator()(BRLCAD::VectorList::Element *element) override {
                list.push_back(element);
                return true;
            }
        };

        Callback b;

        list.clear();
        vl.Iterate(b);
        lists[objectName] = list;
    }
    return lists;
}

GeometryOperationsManager *Document::getGeometryOperationsManager() const {
    return geometryOperationsManager;
}

Display *Document::getDisplay() const {
    return display;
}
