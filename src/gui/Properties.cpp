
#include <include/CollapsibleWidget.h>
#include "Properties.h"
#include <Globals.h>
#include <iostream>
#include "Utils.h"

Properties::Properties(Document & document) : document(document), object(nullptr), current(nullptr) {
    nameWidget = new QLabel(this);
    nameWidget->setWordWrap(true);
    nameWidget->setObjectName("properties-nameWidget");

    fullPathWidget = new QLabel(this);
    fullPathWidget->setWordWrap(true);
    fullPathWidget->setObjectName("properties-fullPathWidget");

    typeSpecificPropertiesArea = new QVBoxWidget(this);

    layout()->addWidget(nameWidget);
    layout()->addWidget(fullPathWidget);
    layout()->addWidget(typeSpecificPropertiesArea);
    getBoxLayout()->addStretch();
}


void Properties::bindObject(const int objectId) {
    this->fullPath = document.getObjectTree()->getFullPathMap()[objectId];
    this->name = fullPath.split("/").last();
    fullPathWidget->setText(QString(fullPath).replace("/"," / "));

    delete object;
    object = document.getDatabase()->Get(fullPath.toUtf8().data());
    objectType = QString(object->Type());

    delete current;
    current = new TypeSpecificProperties(document, object, objectId);
    typeSpecificPropertiesArea->addWidget(current);

    QString nameType = "<font color='$Color-PropertiesObjectNameText'>"+name+"</font><font color='$Color-DefaultFontColor'> ( "
                        "<font color='$Color-PropertiesObjectTypeText'>"+breakStringAtCaps(objectType)+"</font><font color='$Color-DefaultFontColor'> )";
    nameWidget->setText(Globals::theme->process(nameType));
}
