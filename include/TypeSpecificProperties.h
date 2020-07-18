
#ifndef RT3_TYPESPECIFICPROPERTIES_H
#define RT3_TYPESPECIFICPROPERTIES_H


#include <brlcad/MemoryDatabase.h>
#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>

class TypeSpecificProperties: public QFrame {
public:
    TypeSpecificProperties(BRLCAD::MemoryDatabase &database, const BRLCAD::Object &object);
    static TypeSpecificProperties * build(BRLCAD::MemoryDatabase& database, BRLCAD::Object &object);

protected:
    BRLCAD::MemoryDatabase& database;
    const BRLCAD::Object &object;
    QVBoxLayout *mainVLayout;
};


#endif //RT3_TYPESPECIFICPROPERTIES_H
