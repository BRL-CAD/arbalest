
#include <include/VectorDataRow.h>
#include <brlcad/Ellipsoid.h>
#include "ObjectDataTable.h"

using namespace BRLCAD;



ObjectDataTable::ObjectDataTable(QWidget *parent) : QVBoxWidget(parent) {

    Ellipsoid * o;

    BRLCAD::Vector3D (BRLCAD::Ellipsoid::*function)(unsigned __int64)const = &Ellipsoid::SemiPrincipalAxis;



    int count = 1;
    for (int i = 0; i < count; i++) {
        VectorDataRow *vectorDataRow = new VectorDataRow(!i, this);
        if(i!=count-1) {
            for(QLineEdit * textBox: vectorDataRow->getTextBoxes()){
                textBox->setStyleSheet("border-bottom-width: 0px");
            }
        }
        addWidget(vectorDataRow);
    }
}




