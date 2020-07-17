
#include <include/CollapsibleWidget.h>
#include "Properties.h"
#include <Globals.h>

Properties::Properties(BRLCAD::ConstDatabase& database) : database(database) {
    nameWidget = new QLabel();
    fullPathWidget = new QLabel();
    fullPathWidget->setWordWrap(true);

    nameWidget->setObjectName("propertyNameWidget");
    fullPathWidget->setObjectName("propertyFullPathWidget");

    QVBoxLayout * _layout = new QVBoxLayout;
    setLayout(_layout);
    _layout->setContentsMargins(0,0,0,0);

    _layout->addWidget(nameWidget);
    _layout->addWidget(fullPathWidget);

    _layout->setSizeConstraint(QLayout::SetMaximumSize);
    nameWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);
    typeProperties = new CollapsibleWidget();
    typeProperties->setTitle("Ellipsoid");

    layout()->addWidget(typeProperties);

    _layout->addStretch();
}

void Properties::bindObject(const QString &fullPath) {
    this->fullPath = fullPath;
    this->name = fullPath.split("/").last();

    ObjectCallback cb(this);
    database.Get(fullPath.toUtf8().data(),cb);

    nameWidget->setText(name);
    QString nameType = "<font color='$Color-SelectedObjectText'>"+name+"</font><font color='$Color-DefaultFontColor'> ( "
               "            <font color='$Color-CollapsibleTitle'>"+objectType+"</font><font color='$Color-DefaultFontColor'> )";
    nameWidget->setText(Globals::theme->process(nameType));
    QString fullPathDisplay = fullPath;
    fullPathWidget->setText("/ "+fullPathDisplay.replace("/"," / "));
    typeProperties->setTitle(objectType);
}

void Properties::ObjectCallback::operator()(const BRLCAD::Object &object) {
    properties->objectType = QString(object.Type());
}
