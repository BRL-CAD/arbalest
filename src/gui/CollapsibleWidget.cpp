
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include "CollapsibleWidget.h"

CollapsibleWidget::CollapsibleWidget(QWidget * parent):QFrame(parent) {
    _layout = new QVBoxLayout;
    setLayout(_layout);
    _layout->setContentsMargins(0,0,0,0);
    setObjectName("properties-collapsibleWidget");

    titleWidget = new QPushButton(this);
    titleWidget->setObjectName("properties-collapsibleTitle");
    _layout->addWidget(titleWidget);
    connect(titleWidget,&QPushButton::clicked, this,[this]() {setExpanded(!this->expanded);});
    widget = nullptr;
}

void CollapsibleWidget::setTitle(QString title) {
    this->title = title;
    this->titleWidget->setText(title+ "   "+(expanded?"▼":"►"));
}

void CollapsibleWidget::setWidget(QWidget *widget) {
    if (_layout->count()>1 && this->widget){
        this->widget->setParent(nullptr);
    }
    _layout->addWidget(widget);
    this->widget = widget;
    widget->setVisible(expanded);
}

void CollapsibleWidget::setExpanded(bool expanded) {
    if (expanded == this->expanded)return;
    if(widget){
        widget->setVisible(expanded);
        updateGeometry();
    }
    this->expanded = expanded;
    setTitle(title);
}
