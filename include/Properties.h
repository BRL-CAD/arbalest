
#ifndef RT3_PROPERTIES_H
#define RT3_PROPERTIES_H


#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <include/CollapsibleWidget.h>
#include <brlcad/Database/Database.h>
#include <brlcad/Database/MemoryDatabase.h>
#include "Document.h"
#include "QVBoxWidget.h"
#include "TypeSpecificProperties.h"

class Document;
class TypeSpecificProperties;
class Properties: public QVBoxWidget{
public:
    explicit Properties(Document & document);
    void bindObject(const unsigned int objectId);
    void rewriteObjectNameAndType();

private:
    QString name, fullPath, objectType;

	// UI components
    Document & document;
    TypeSpecificProperties * current;
    BRLCAD::Object * object;
    QLabel * nameWidget;
    QLabel * fullPathWidget;
    QVBoxWidget * typeSpecificPropertiesArea;
};


#endif //RT3_PROPERTIES_H
