
#include <QtWidgets/QHBoxLayout>
#include "QHBoxWidget.h"

QHBoxWidget::QHBoxWidget(QWidget *parent, Qt::WindowFlags f) : QFrame(parent, f) {
    boxLayout = new QHBoxLayout(this);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->setSpacing(0);
    setLayout(boxLayout);
}

QHBoxWidget::QHBoxWidget(QWidget* parent, QWidget* child):QHBoxWidget(parent)
{
    addWidget(child);
}

QHBoxLayout *QHBoxWidget::getBoxLayout() {
    return boxLayout;
}

void QHBoxWidget::addWidget(QWidget* widget) {
    boxLayout->addWidget(widget);
}
