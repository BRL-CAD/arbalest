
#ifndef RT3_PROPERTIES_H
#define RT3_PROPERTIES_H


#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <include/CollapsibleWidget.h>
#include <brlcad/Database.h>
#include <brlcad/MemoryDatabase.h>
#include "Document.h"
#include "QVBoxWidget.h"

class Document;
class Properties: public QVBoxWidget{
public:
    explicit Properties(Document & document);
    void bindObject(const int objectId);

private:
    QString name, fullPath, objectType;

	// UI components
    Document & document;
    QLabel * nameWidget;
    QLabel * fullPathWidget;
    CollapsibleWidget * typeSpecificPropertiesArea;
};


#endif //RT3_PROPERTIES_H
