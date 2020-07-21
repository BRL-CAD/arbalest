
#include "DragEditLineEdit.h"
#include <QApplication>
#include <QDesktopWidget>

DragEditLineEdit::DragEditLineEdit(const QString &string, QWidget *parent) : DragEditLineEdit(parent) {
    setText(string);
}

DragEditLineEdit::DragEditLineEdit(QWidget *parent) : QLineEdit(parent) {
    setCursor(Qt::SizeVerCursor);
}



void DragEditLineEdit::mouseMoveEvent(QMouseEvent *event) {

    if(initialY != -1 && (event->buttons() & Qt::MouseButton::LeftButton)) {
        int globalY = event->globalY();
        int deltaY = initialY-globalY;
        int change = 0;
        if (deltaY > initialTolerance){
            change = deltaY - initialTolerance;
        }
        else if(deltaY < -initialTolerance){
            change = deltaY + initialTolerance;
        }
        if(abs(deltaY)>initialTolerance) {
            if (abs(globalY-lastEmittedGlobalY)*100>screenHeight){
                lastEmittedGlobalY = globalY;
                setText(QString::number(map(change)));
                emit textEdited(text());
            }
        }
    }

    QLineEdit::mouseMoveEvent(event);
}

void DragEditLineEdit::mousePressEvent(QMouseEvent *event) {
    initialY = event->globalY();
    initialValue = text().toDouble();
    lastEmittedGlobalY = initialY;
    QLineEdit::mousePressEvent(event);
    screenHeight = QApplication::desktop()->screenGeometry().height();
}

void DragEditLineEdit::mouseReleaseEvent(QMouseEvent *event) {
    initialY = -1;
    QLineEdit::mouseReleaseEvent(event);
}

double DragEditLineEdit::map(int cursorValue){
    double val = initialValue + cursorValue * (initialValue * 0.01 + 0.02);
    return val;
}