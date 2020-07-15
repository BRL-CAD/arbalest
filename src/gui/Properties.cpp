
#include "Properties.h"

Properties::Properties()  {
    nameWidget = new QLabel();
    fullPathWidget = new QLabel();

    nameWidget->setObjectName("propertyNameWidget");
    fullPathWidget->setObjectName("propertyFullPathWidget");

    QVBoxLayout * _layout = new QVBoxLayout;
    setLayout(_layout);

    _layout->addWidget(nameWidget);
    _layout->addWidget(fullPathWidget);
    _layout->addStretch();
}

void Properties::bindObject(const QString &fullPath) {
    this->fullPath = fullPath;
    this->name = fullPath.split("/").last();

    nameWidget->setText(name);
    fullPathWidget->setText(fullPath);
}
