
#ifndef RT3_COLLAPSIBLEWIDGET_H
#define RT3_COLLAPSIBLEWIDGET_H


#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

class CollapsibleWidget: public QFrame {
public:
    CollapsibleWidget();
    void setTitle(QString title);
    void setWidget(QWidget *widget);

private:
    QLabel * title;
    QWidget * widget;
    QVBoxLayout *_layout;
};


#endif //RT3_COLLAPSIBLEWIDGET_H
