
#include <include/CollapsibleWidget.h>
#include "Properties.h"
#include "TypeSpecificProperties.h"
#include <Globals.h>
#include <iostream>
#include "Utils.h"

Properties::Properties(Document & document) : document(document) {

    nameWidget = new QLabel(this);
    nameWidget->setWordWrap(true);
    nameWidget->setObjectName("properties-nameWidget");

    fullPathWidget = new QLabel(this);
    fullPathWidget->setWordWrap(true);
    fullPathWidget->setObjectName("properties-fullPathWidget");

    typeSpecificPropertiesArea = new CollapsibleWidget(this);

    layout()->addWidget(nameWidget);
    layout()->addWidget(fullPathWidget);
    layout()->addWidget(typeSpecificPropertiesArea);
    getBoxLayout()->addStretch();
}


void Properties::bindObject(const int objectId) {
    this->fullPath = document.getObjectTree()->getFullPathMap()[objectId];
    this->name = fullPath.split("/").last();
    fullPathWidget->setText(QString(fullPath).replace("/"," / "));

    static BRLCAD::Object* object = nullptr;
    delete object;
	object = document.getDatabase()->Get(fullPath.toUtf8().data());
    objectType = QString(object->Type());

    static TypeSpecificProperties * current = nullptr;
    delete current;
    current = new TypeSpecificProperties(document, object, objectId);
    typeSpecificPropertiesArea->setWidget(current);

    QString nameType = "<font color='$Color-PropertiesObjectNameText'>"+name+"</font><font color='$Color-DefaultFontColor'> ( "
                        "<font color='$Color-PropertiesObjectTypeText'>"+breakStringAtCaps(objectType)+"</font><font color='$Color-DefaultFontColor'> )";
    nameWidget->setText(Globals::theme->process(nameType));
    typeSpecificPropertiesArea->setTitle(breakStringAtCaps(objectType));
}
