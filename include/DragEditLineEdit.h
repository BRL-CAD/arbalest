
#ifndef RT3_DRAGEDITLINEEDIT_H
#define RT3_DRAGEDITLINEEDIT_H


#include <QtWidgets/QLineEdit>
#include <QMouseEvent>

class DragEditLineEdit : public QLineEdit {
public:
    explicit DragEditLineEdit(const QString &string, QWidget *parent = nullptr);
    explicit DragEditLineEdit(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int initialY;
    int lastEmittedGlobalY;
    double map(int cursorValue);
    const int initialTolerance = 20;
    double initialValue;
    int screenHeight;
};


#endif //RT3_DRAGEDITLINEEDIT_H
