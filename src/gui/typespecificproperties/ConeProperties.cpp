
#include <QtWidgets/QLabel>
#include <brlcad/Cone.h>
#include <QtOpenGL/QtOpenGL>
#include "ConeProperties.h"

ConeProperties::ConeProperties(BRLCAD::MemoryDatabase &database, BRLCAD::Cone &o)
        : TypeSpecificProperties(database, object) {


    QLabel * g = new QLabel(QString::number(o.Height().coordinates[0])+" "+QString::number(o.Height().coordinates[1])+" "+QString::number(o.Height().coordinates[2]));
    this->mainVLayout->addWidget(g);


    QTableWidget * tableWidget = new QTableWidget(12, 3, this);
    tableWidget->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Maximum);
    this->mainVLayout->addWidget(tableWidget);

}
