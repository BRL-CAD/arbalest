
#include <include/CollapsibleWidget.h>
#include "Properties.h"
#include "TypeSpecificProperties.h"
#include <Globals.h>
#include <iostream>

QString breakStringAtCaps(const QString& in)
{
    QString newName;
    for(int i = 0; i < in.size(); i++)
    {
        if(in[i].isUpper() && i != 0) newName += " ";
        newName += in[i];
    }
    return newName;
}

Properties::Properties(Document & document) : document(document) {
    QVBoxLayout * _layout = new QVBoxLayout;
    setLayout(_layout);
    _layout->setContentsMargins(0,0,0,0);

    nameWidget = new QLabel(this);
    nameWidget->setWordWrap(true);
    nameWidget->setObjectName("properties-nameWidget");

    fullPathWidget = new QLabel(this);
    fullPathWidget->setWordWrap(true);
    fullPathWidget->setObjectName("properties-fullPathWidget");

    typeSpecificPropertiesArea = new CollapsibleWidget(this);

    _layout->addWidget(nameWidget);
    _layout->addWidget(fullPathWidget);
    _layout->addWidget(typeSpecificPropertiesArea);
    _layout->addStretch();
}


void Properties::bindObject(const QString &fullPath) {
    this->fullPath = fullPath;
    this->name = fullPath.split("/").last();
    fullPathWidget->setText("/ "+QString(fullPath).replace("/"," / "));

    BRLCAD::Object * object = document.getDatabase()->Get(fullPath.toUtf8().data());

    objectType = QString(object->Type());

    static TypeSpecificProperties * current = nullptr;
    delete current;
    current = new TypeSpecificProperties(document,object);
    typeSpecificPropertiesArea->setWidget(current);

    QString nameType = "<font color='$Color-SelectedObjectText'>"+name+"</font><font color='$Color-DefaultFontColor'> ( "
                        "<font color='$Color-SelectedObjectTypeText'>"+breakStringAtCaps(objectType)+"</font><font color='$Color-DefaultFontColor'> )";
    nameWidget->setText(Globals::theme->process(nameType));
    typeSpecificPropertiesArea->setTitle(breakStringAtCaps(objectType));
}

void Properties::ObjectCallback::operator()(BRLCAD::Object &object) {
}
