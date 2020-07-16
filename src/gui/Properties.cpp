
#include "Properties.h"

Properties::Properties()  {
    nameWidget = new QLabel();
    fullPathWidget = new QLabel();
    fullPathWidget->setWordWrap(true);

    nameWidget->setObjectName("propertyNameWidget");
    fullPathWidget->setObjectName("propertyFullPathWidget");

    QVBoxLayout * _layout = new QVBoxLayout;
    setLayout(_layout);

    _layout->addWidget(nameWidget);
    _layout->addWidget(fullPathWidget);


    Section* section = new Section("Elipsaid  ►", 50, this);
    auto* anyLayout = new QVBoxLayout();
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    anyLayout->addWidget(new QLabel("Some Text in Section", section));
    section->setContentLayout(*anyLayout);
    layout()->addWidget(section);

    _layout->addStretch();
}

void Properties::bindObject(const QString &fullPath) {
    this->fullPath = fullPath;
    this->name = fullPath.split("/").last();

    nameWidget->setText(name);
    QString fullPathDisplay = fullPath;
    fullPathWidget->setText("/ "+fullPathDisplay.replace("/"," / "));





}
