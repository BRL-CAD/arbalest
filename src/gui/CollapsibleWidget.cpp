
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <include/QHBoxWidget.h>
#include "CollapsibleWidget.h"

CollapsibleWidget::CollapsibleWidget(QWidget * parent):QFrame(parent) {
    _layout = new QVBoxLayout;
    setLayout(_layout);
    _layout->setContentsMargins(0,0,0,0);
    setObjectName("properties-collapsibleWidget");

    QHBoxWidget * titleBar = new QHBoxWidget(this);
    titleLabel = new QPushButton(titleBar);
    collapseIcon = new QPushButton(titleBar);
    titleLabel->setObjectName("properties-collapsibleTitle");
    collapseIcon->setObjectName("properties-collapsibleIcon");
    titleBar->addWidget(titleLabel);
    titleBar->addWidget(collapseIcon);
    titleBar->getBoxLayout()->addStretch();
    _layout->addWidget(titleBar);
    connect(titleLabel, &QPushButton::clicked, this, [this]() {setExpanded(!this->expanded);});
    connect(collapseIcon, &QPushButton::clicked, this, [this]() {setExpanded(!this->expanded);});
    widget = nullptr;
}

void CollapsibleWidget::setTitle(QString title) {
    this->title = title;
    this->titleLabel->setText(title + "   " );
    this->collapseIcon->setText(expanded ? "▼" : "►");
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
