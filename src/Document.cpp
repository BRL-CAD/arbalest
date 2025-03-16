//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <ArbDisplay.h>
#include <brlcad/Database/Torus.h>


Document::Document(const int documentId, const QString *filePath) : documentId(documentId)
{
    if (filePath != nullptr) this->filePath = new QString(*filePath);
    database =  new BRLCAD::MemoryDatabase();
    if (filePath != nullptr) {
        if (!database->Load(filePath->toUtf8().data())) throw std::runtime_error("Failed to open file");
    }

    modified = false;
    objectTree = new ObjectTree(database);
    properties = new Properties(*this);
    geometryRenderer = new GeometryRenderer(this);
    objectTreeWidget = new ObjectTreeWidget(this);
    displayGrid = new ArbDisplayGrid(this);

    displayGrid->forceRerenderAllArbDisplays();

    raytraceWidget = new RaytraceView(this);
}

Document::~Document()
{
    delete database;
}

void Document::modifyObject(BRLCAD::Object *newObject)
{
    modified = true;
    database->Set(*newObject);
    QString objectName = newObject->Name();
    getObjectTree()->traverseSubTree(0,false,[this, objectName](int objectId) {
        if (getObjectTree()->getNameMap()[objectId] == objectName) geometryRenderer->clearObject(objectId);
        return true;
    });
    geometryRenderer->refreshForVisibilityAndSolidChanges();
    for (ArbDisplay * display : displayGrid->getArbDisplays())display->forceRerenderFrame();
}

bool Document::isModified()
{
    return modified;
}

bool Document::Add(const BRLCAD::Object& object)
{
    modified = true;
    return database->Add(object);
}

bool Document::Save(const char* fileName)
{
    modified = false;
    return database->Save(fileName);
}

void Document::getBRLCADConstObject(const QString& objectName, const std::function<void(const BRLCAD::Object&)>& func) const
{
    database->Get(objectName.toUtf8(), [func](const BRLCAD::Object& object){func(object);});
}

void Document::getBRLCADObject(const QString& objectName, const std::function<void(BRLCAD::Object&)>& func)
{
    database->Get(objectName.toUtf8(), func);
    modified = true;
}

ArbDisplay* Document::getArbDisplay()
{
    return displayGrid->getActiveArbDisplay();
}
