
#ifndef RT3_TYPESPECIFICPROPERTIES_H
#define RT3_TYPESPECIFICPROPERTIES_H


#include <brlcad/MemoryDatabase.h>
#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>
#include "Document.h"

class TypeSpecificProperties: public QFrame {
public:
    TypeSpecificProperties(Document &document, BRLCAD::Object *object);

protected:
    Document& document;
    BRLCAD::Object *object;
    QVBoxLayout *l;
};


#endif //RT3_TYPESPECIFICPROPERTIES_H
