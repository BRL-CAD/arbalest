
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include "CollapsibleWidget.h"

CollapsibleWidget::CollapsibleWidget() {
    _layout = new QVBoxLayout;
    setLayout(_layout);
    _layout->setContentsMargins(0,0,0,0);

    title = new QLabel(this);
    title->setObjectName("collapsibleTitle");
    _layout->addWidget(title);
}

void CollapsibleWidget::setTitle(QString title) {
    this->title->setText(title);
}

void CollapsibleWidget::setWidget(QWidget *widget) {
    if (this->widget){
        _layout->removeWidget(this->widget);
    }
    _layout->addWidget(widget);
    this->widget = widget;
}
