//
// Created by Sadeep on 11-Jun.
//

#include <Document.h>
#include <iostream>
#include <ObjectTreeWidget.h>

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
    display = new Display(this);
    objectTreeWidget = new ObjectTreeWidget(this);
    display->forceRerenderFrame();

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
    delete display;
}
