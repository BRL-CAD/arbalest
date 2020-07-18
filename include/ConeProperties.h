
#ifndef RT3_CONEPROPERTIES_H
#define RT3_CONEPROPERTIES_H


#include "TypeSpecificProperties.h"
#include <brlcad/Cone.h>

class ConeProperties: public TypeSpecificProperties {
public:
    ConeProperties(BRLCAD::MemoryDatabase &database, BRLCAD::Cone &object);
};


#endif //RT3_CONEPROPERTIES_H
