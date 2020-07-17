
#ifndef RT3_CONESPECIFICPROPERTIES_H
#define RT3_CONESPECIFICPROPERTIES_H


#include "TypeSpecificProperties.h"

class ConeSpecificProperties: public TypeSpecificProperties {
public:
    ConeSpecificProperties(BRLCAD::ConstDatabase &database, const BRLCAD::Object &object);
};


#endif //RT3_CONESPECIFICPROPERTIES_H
