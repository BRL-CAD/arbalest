//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include<Display.h>


Document::Document(const int documentId, const QString *filePath) : documentId(documentId) {
    if (filePath != nullptr) this->filePath = new QString(*filePath);
    database =  new BRLCAD::MemoryDatabase();
    if (filePath != nullptr) {
        if (!database->Load(filePath->toUtf8().data()))
        {
            throw std::exception("Failed to open file");
        }
    }

    objectTree = new ObjectTree(database);
    properties = new Properties(*this);
    geometryRenderer = new GeometryRenderer(this);
    objectTreeWidget = new ObjectTreeWidget(this);
    displayGrid = new DisplayGrid(this);

    displayGrid->forceRerenderAllDisplays();

    raytraceWidget = new RaytraceView(this);

//
//    int t0 = time(NULL);
//    for (int i=0;i<1000;i++){
//        objectTree->traverseSubTree(0,false,[this](int objectId){
//            if(objectId%2)objectTreeWidget->getObjectIdTreeWidgetItemMap()[objectId]->setForeground(0, QBrush(QColor(Qt::green)));
//        });
//    }
//    printf("time = %d secs\n",  time(NULL) - t0);
}

Document::~Document() {
    delete database;
}

void Document::modifyObject(BRLCAD::Object *newObject) {
    database->Set(*newObject);
    QString objectName = newObject->Name();
    getObjectTree()->traverseSubTree(0,false,[this, objectName]
    (int objectId){
        if (getObjectTree()->getNameMap()[objectId] == objectName){
            geometryRenderer->clearObject(objectId);
        }
        return true;
    }
    );
    geometryRenderer->refreshForVisibilityAndSolidChanges();
    for (Display * display : displayGrid->getDisplays())display->forceRerenderFrame();
}


void Document::modifyObjectNoSet(BRLCAD::Object *newObject) {
    QString objectName = newObject->Name();
    getObjectTree()->traverseSubTree(0,false,[this, objectName]
                                             (int objectId){
                                         if (getObjectTree()->getNameMap()[objectId] == objectName){
                                             geometryRenderer->clearObject(objectId);
                                         }
                                         return true;
                                     }
    );
    geometryRenderer->refreshForVisibilityAndSolidChanges();
    for (Display * display : displayGrid->getDisplays())display->forceRerenderFrame();
}
Display* Document::getDisplay()
{
    return displayGrid->getActiveDisplay();
}