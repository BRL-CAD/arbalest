
#ifndef RT3_MATRIXTRANSFORMWIDGET_H
#define RT3_MATRIXTRANSFORMWIDGET_H


#include <include/Document.h>
#include "DataRow.h"

class MatrixTransformWidget : public DataRow{
public:
    enum TransformType {Translate, Rotate, Scale};
    MatrixTransformWidget(Document * document, size_t childObjectId, TransformType transformType);
private:
    static QHash<size_t,QWidget*> widgets;
};


#endif //RT3_MATRIXTRANSFORMWIDGET_H
