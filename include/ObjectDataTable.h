
#ifndef RT3_OBJECTDATATABLE_H
#define RT3_OBJECTDATATABLE_H


#include <QtWidgets/QFrame>
#include "QVBoxWidget.h"

#include <brlcad/Ellipsoid.h>

class ObjectDataTable : public QVBoxWidget {
public:
    explicit ObjectDataTable(QWidget* parent = nullptr);

    template <typename T> void initialize(BRLCAD::Vector3D (T::*function)(unsigned __int64)const){

    }

};


#endif //RT3_OBJECTDATATABLE_H
