
#ifndef RT3_PROPERTIES_H
#define RT3_PROPERTIES_H


#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <include/CollapsibleWidget.h>
#include <brlcad/ConstDatabase.h>

class Properties: public QFrame{
public:
    explicit Properties(BRLCAD::ConstDatabase& database);
    void bindObject(const QString &fullPath);
private:
    class ObjectCallback: public BRLCAD::ConstDatabase::ObjectCallback{
    public:
        explicit ObjectCallback(Properties *properties) : properties(properties) {}
        void operator()(const BRLCAD::Object& object) override;

    private:
        Properties *properties;
    };

    BRLCAD::ConstDatabase & database;
    QString name, fullPath, objectType;
    QLabel * nameWidget;
    QLabel * fullPathWidget;
    CollapsibleWidget * typeProperties;
};


#endif //RT3_PROPERTIES_H
