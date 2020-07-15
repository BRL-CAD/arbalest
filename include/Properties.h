
#ifndef RT3_PROPERTIES_H
#define RT3_PROPERTIES_H


#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

class Properties: public QFrame{
public:
    Properties();
    void bindObject(const QString &fullPath);
//private:
    QString name, fullPath;

    QLabel * nameWidget;
    QLabel * fullPathWidget;
};


#endif //RT3_PROPERTIES_H
