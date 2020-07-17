
#include <include/CollapsibleWidget.h>
#include "Properties.h"
#include "TypeSpecificProperties.h"
#include <Globals.h>
#include <iostream>

Properties::Properties(BRLCAD::ConstDatabase& database) : database(database) {
    QVBoxLayout * _layout = new QVBoxLayout;
    setLayout(_layout);
    _layout->setContentsMargins(0,0,0,0);
    _layout->setSizeConstraint(QLayout::SetMinimumSize);

    nameWidget = new QLabel();
    nameWidget->setObjectName("propertyNameWidget");

    fullPathWidget = new QLabel();
    fullPathWidget->setWordWrap(true);
    typeSpecificPropertiesArea = new CollapsibleWidget();

    _layout->addWidget(nameWidget);
    _layout->addWidget(fullPathWidget);
    _layout->addWidget(typeSpecificPropertiesArea);
    _layout->addStretch();
}


void Properties::bindObject(const QString &fullPath) {
    this->fullPath = fullPath;
    this->name = fullPath.split("/").last();

    ObjectCallback objectCallback(this);
    database.Get(fullPath.toUtf8().data(),objectCallback);

    nameWidget->setText(name);
    QString nameType = "<font color='$Color-SelectedObjectText'>"+name+"</font><font color='$Color-DefaultFontColor'> ( "
                        "<font color='$Color-CollapsibleTitle'>"+objectType+"</font><font color='$Color-DefaultFontColor'> )";
    nameWidget->setText(Globals::theme->process(nameType));

    fullPathWidget->setText("/ "+QString(fullPath).replace("/"," / "));
    typeSpecificPropertiesArea->setTitle(objectType);
}

void Properties::ObjectCallback::operator()(const BRLCAD::Object &object) {
    properties->objectType = QString(object.Type());
    auto k = TypeSpecificProperties::build(properties->database,object);
    properties->typeSpecificPropertiesArea->setWidget(k);
}
