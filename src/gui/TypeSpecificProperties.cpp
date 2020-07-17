
#include <QtWidgets/QVBoxLayout>
#include <include/ConeSpecificProperties.h>
#include "TypeSpecificProperties.h"

TypeSpecificProperties::TypeSpecificProperties(BRLCAD::ConstDatabase &database, const BRLCAD::Object &object)
        : database(database), object(object) {

    mainVLayout = new QVBoxLayout;
    mainVLayout->setContentsMargins(0,0,0,0);
    setLayout(mainVLayout);
}

TypeSpecificProperties *TypeSpecificProperties::build(BRLCAD::ConstDatabase &database, const BRLCAD::Object &object) {
    return new ConeSpecificProperties(database,object);
}
