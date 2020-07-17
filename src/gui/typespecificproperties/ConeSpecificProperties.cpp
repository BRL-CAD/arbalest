
#include <QtWidgets/QLabel>
#include "ConeSpecificProperties.h"

ConeSpecificProperties::ConeSpecificProperties(BRLCAD::ConstDatabase &database, const BRLCAD::Object &object)
        : TypeSpecificProperties(database, object) {
    QLabel * g = new QLabel("Goliath.c");
    this->mainVLayout->addWidget(g);
}
