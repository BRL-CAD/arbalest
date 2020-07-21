
#ifndef RT3_COLLAPSIBLEWIDGET_H
#define RT3_COLLAPSIBLEWIDGET_H


#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>

class CollapsibleWidget: public QFrame {
    Q_OBJECT
public:
    CollapsibleWidget(QWidget * parent = nullptr);
    void setTitle(QString title);
    void setWidget(QWidget *widget);

private:
    QString title;
    QPushButton * titleLabel;
    QPushButton * collapseIcon;
    QWidget * widget;
    QVBoxLayout *_layout;
    bool expanded = true;
private slots:
    void setExpanded(bool expanded);
};


#endif //RT3_COLLAPSIBLEWIDGET_H
