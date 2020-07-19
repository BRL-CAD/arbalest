
#include <QtWidgets/QVBoxLayout>
#include <include/ConeProperties.h>
#include <brlcad/Cone.h>
#include <QtOpenGL/QtOpenGL>
#include "TypeSpecificProperties.h"
#include "QHBoxWidget.h"

#include <QStyledItemDelegate>
#include <include/QVBoxWidget.h>
#include <include/DataRow.h>


TypeSpecificProperties::TypeSpecificProperties(BRLCAD::MemoryDatabase &database, const BRLCAD::Object &object)
        : database(database), object(object) {

    mainVLayout = new QVBoxLayout(this);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainVLayout);
    setObjectName("properties-TypeSpecificProperties");

    DataRow *dataRow =     new DataRow(3,0,this);

    mainVLayout->addWidget(dataRow);
    mainVLayout->addStretch(1);
}

TypeSpecificProperties *TypeSpecificProperties::build(BRLCAD::MemoryDatabase &database, BRLCAD::Object &object) {
    if (QString(object.Type()) == "Cone") {
        return new ConeProperties(database, dynamic_cast<BRLCAD::Cone &>(object));
    }

    return new TypeSpecificProperties(database, object);
}










//
//for(int y=0;y<Y;y++)for(int x=0;x<X;x++) {
//tableWidget->setItem(y, x, new QTableWidgetItem);
//tableWidget->item(y, x)->setBackground(Qt::white);
//}
