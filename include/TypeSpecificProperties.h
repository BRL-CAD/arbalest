
#ifndef RT3_TYPESPECIFICPROPERTIES_H
#define RT3_TYPESPECIFICPROPERTIES_H


#include <brlcad/ConstDatabase.h>
#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>

class TypeSpecificProperties: public QFrame {
public:
    TypeSpecificProperties(BRLCAD::ConstDatabase &database, const BRLCAD::Object &object);
    static TypeSpecificProperties * build(BRLCAD::ConstDatabase& database,const BRLCAD::Object &object);

protected:
    BRLCAD::ConstDatabase& database;
    const BRLCAD::Object &object;
    QVBoxLayout *mainVLayout;
};


#endif //RT3_TYPESPECIFICPROPERTIES_H
