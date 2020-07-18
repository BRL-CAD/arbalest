
#ifndef RT3_PROPERTIES_H
#define RT3_PROPERTIES_H


#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <include/CollapsibleWidget.h>
#include <brlcad/Database.h>
#include <brlcad/MemoryDatabase.h>

class Properties: public QFrame{
public:
    explicit Properties(BRLCAD::MemoryDatabase& database);
    void bindObject(const QString &fullPath);

private:

    class ObjectCallback: public BRLCAD::Database::ObjectCallback{
    public:
        explicit ObjectCallback(Properties *properties) : properties(properties) {}
        void operator()(BRLCAD::Object& object) override;
    private:
        Properties *properties;
    };

    BRLCAD::MemoryDatabase & database;
    QString name, fullPath, objectType;
    QLabel * nameWidget;
    QLabel * fullPathWidget;
    CollapsibleWidget * typeSpecificPropertiesArea;
};


#endif //RT3_PROPERTIES_H
