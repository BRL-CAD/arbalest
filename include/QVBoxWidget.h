
#ifndef RT3_QVBOXWIDGET_H
#define RT3_QVBOXWIDGET_H


#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>

class QVBoxWidget : public QFrame{
public:
    QVBoxWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    QVBoxLayout *getBoxLayout();
    void addWidget(QWidget* widget);
private:
    QVBoxLayout *boxLayout;
};


#endif //RT3_QHVOXWIDGET_H
