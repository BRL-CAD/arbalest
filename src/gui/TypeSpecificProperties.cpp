
#include <QtWidgets/QVBoxLayout>
#include <include/ConeProperties.h>
#include <brlcad/Cone.h>
#include <QtOpenGL/QtOpenGL>
#include "TypeSpecificProperties.h"

TypeSpecificProperties::TypeSpecificProperties(BRLCAD::MemoryDatabase &database, const BRLCAD::Object &object)
        : database(database), object(object) {

    mainVLayout = new QVBoxLayout;
    mainVLayout->setContentsMargins(0,0,0,0);
    setLayout(mainVLayout);
    setObjectName("properties-TypeSpecificProperties");



    QTableWidget * tableWidget = new QTableWidget(12, 3, this);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->mainVLayout->addWidget(tableWidget);

}

TypeSpecificProperties *TypeSpecificProperties::build(BRLCAD::MemoryDatabase &database, BRLCAD::Object &object) {
    if (QString(object.Type()) == "Cone"){
        return new ConeProperties(database, dynamic_cast<BRLCAD::Cone&>(object));
    }

    return new TypeSpecificProperties(database, object);
}
