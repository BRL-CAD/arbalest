//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include<Viewport.h>
#include <brlcad/Database/Torus.h>


Document::Document(const int documentId, const QString *filePath) : documentId(documentId) {
    if (filePath != nullptr) this->filePath = new QString(*filePath);
    database =  new BRLCAD::MemoryDatabase();
    if (filePath != nullptr) {
        if (!database->Load(filePath->toUtf8().data()))
        {
            throw std::runtime_error("Failed to open file");
        }
    }

    modified = false;
    objectTree = new ObjectTree(database);
    properties = new Properties(*this);
    geometryRenderer = new GeometryRenderer(this);
    objectTreeWidget = new ObjectTreeWidget(this);
    displayGrid = new ViewportGrid(this);

    displayGrid->forceRerenderAllViewports();

    raytraceWidget = new RaytraceView(this);
}

Document::~Document() {
    delete database;
}

void Document::modifyObject(BRLCAD::Object *newObject) {
    modified = true;
    database->Set(*newObject);
    QString objectName = newObject->Name();
    getObjectTree()->traverseSubTree(0, false, [this, objectName](size_t objectId) {
        if (getObjectTree()->getItems()[objectId]->getName() == objectName)
            geometryRenderer->clearObject(objectId);
        return true;
    });
    geometryRenderer->refreshForVisibilityAndSolidChanges();
    for (Viewport * display : displayGrid->getViewports())display->forceRerenderFrame();
}

bool Document::isModified() {
    return modified;
}

bool Document::AddObject(const BRLCAD::Object& object, const bool isVisible) {
    bool ret = database->Add(object);
    if (ret) {
        modified = true;
        size_t objectId = getObjectTree()->addTopObject(QString(object.Name()));
        getObjectTree()->changeVisibilityState(objectId, isVisible);
        getObjectTreeWidget()->build(objectId);
        getObjectTreeWidget()->refreshItemTextColors();
        getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        getViewportGrid()->forceRerenderAllViewports();
    }
    return ret;
}

bool Document::Save(const char* fileName) {
    modified = false;
    return database->Save(fileName);
}

void Document::getBRLCADConstObject(const QString& objectName, const std::function<void(const BRLCAD::Object&)>& func) const {
    database->Get(objectName.toUtf8(), [func](const BRLCAD::Object& object){func(object);});
}

void Document::getBRLCADObject(const QString& objectName, const std::function<void(BRLCAD::Object&)>& func) {
    database->Get(objectName.toUtf8(), func);
    modified = true;
}

Viewport* Document::getViewport()
{
    return displayGrid->getActiveViewport();
}
