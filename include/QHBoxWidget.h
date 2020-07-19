
#ifndef RT3_QHBOXWIDGET_H
#define RT3_QHBOXWIDGET_H


#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>

class QHBoxWidget : public QFrame{
public:
    QHBoxWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    QHBoxLayout *getBoxLayout();
    void addWidget(QWidget* widget);
private:
    QHBoxLayout *boxLayout;
};


#endif //RT3_QHBOXWIDGET_H
