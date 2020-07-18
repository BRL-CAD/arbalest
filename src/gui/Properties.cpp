
#include <include/CollapsibleWidget.h>
#include "Properties.h"
#include "TypeSpecificProperties.h"
#include <Globals.h>
#include <iostream>

Properties::Properties(BRLCAD::MemoryDatabase& database) : database(database) {
    QVBoxLayout * _layout = new QVBoxLayout;
    setLayout(_layout);
    _layout->setContentsMargins(0,0,0,0);

    nameWidget = new QLabel(this);
    nameWidget->setWordWrap(true);
    nameWidget->setObjectName("properties-NameWidget");

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

    ObjectCallback objectCallback(this);
    database.Get(fullPath.toUtf8().data(),objectCallback);
    QString nameType = "<font color='$Color-SelectedObjectText'>"+name+"</font><font color='$Color-DefaultFontColor'> ( "
                        "<font color='$Color-CollapsibleTitle'>"+objectType+"</font><font color='$Color-DefaultFontColor'> )";
    nameWidget->setText(Globals::theme->process(nameType));
    typeSpecificPropertiesArea->setTitle(objectType);
}

void Properties::ObjectCallback::operator()(BRLCAD::Object &object) {
    properties->objectType = QString(object.Type());
    static TypeSpecificProperties * current = nullptr;
    delete current;
    current = TypeSpecificProperties::build(properties->database,object);
    properties->typeSpecificPropertiesArea->setWidget(current);
}
