
#ifndef RT3_MATRIXTRANSFORMWIDGET_H
#define RT3_MATRIXTRANSFORMWIDGET_H


#include <include/Document.h>
#include "DataRow.h"

class MatrixTransformWidget : public DataRow{
public:
    enum TransformType {Translate, Rotate, Scale};
    MatrixTransformWidget(Document * document, int childObjectId, TransformType transformType );
private:
    static QHash<int,QWidget*> widgets;
};


#endif //RT3_MATRIXTRANSFORMWIDGET_H
