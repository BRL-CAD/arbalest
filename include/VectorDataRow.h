
#ifndef RT3_VECTORDATAROW_H
#define RT3_VECTORDATAROW_H


#include <include/DataRow.h>

class VectorDataRow : public DataRow{
public:
    VectorDataRow(bool hasHeader, QWidget *parent);
};


#endif //RT3_VECTORDATAROW_H
