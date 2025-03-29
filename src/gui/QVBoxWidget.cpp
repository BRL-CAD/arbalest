
#include <QtWidgets/QVBoxLayout>
#include "QVBoxWidget.h"

QVBoxWidget::QVBoxWidget(QWidget *parent, Qt::WindowFlags f) : QFrame(parent, f) {
    boxLayout = new QVBoxLayout(this);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->setSpacing(0);
    // boxLayout->setMargin(0);
    setLayout(boxLayout);
}

QVBoxWidget::QVBoxWidget(QWidget* parent, QWidget* child) : QVBoxWidget(parent)
{
    addWidget(child);
}

QVBoxLayout *QVBoxWidget::getBoxLayout() {
    return boxLayout;
}

void QVBoxWidget::addWidget(QWidget* widget) {
    boxLayout->addWidget(widget);
}
