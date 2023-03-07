
#ifndef RT3_OBJECTDATATABLE_H
#define RT3_OBJECTDATATABLE_H


#include <QtWidgets/QFrame>
#include "QVBoxWidget.h"

#include <brlcad/Database/Ellipsoid.h>

class ObjectDataTable : public QVBoxWidget {
public:
    explicit ObjectDataTable(QWidget* parent = nullptr);

    template <typename T> void initialize(BRLCAD::Vector3D (T::*function)(size_t)const){

    }

};


#endif //RT3_OBJECTDATATABLE_H
