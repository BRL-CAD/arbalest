#ifndef UTILS_ARBALEST_H
#define UTILS_ARBALEST_H

#include <brlcad/Combination.h>
#include <brlcad/MemoryDatabase.h>
#include <functional>
#include <utility>
#include <QtWidgets/QLabel>
#include "brlcad/cicommon.h"
#include "QVBoxWidget.h"
#include "QHBoxWidget.h"

BRLCAD::Vector3D operator+(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator-(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator/(const BRLCAD::Vector3D& a, const int& b);
double vector3DLength(const BRLCAD::Vector3D& a);

QString breakStringAtCaps(const QString& in);


struct ColorInfo {
    float red, green, blue;
    bool hasColor;
};

const double * getLeafMatrix(BRLCAD::Combination::TreeNode& node, const QString& name);
void setLeafMatrix(BRLCAD::Combination::TreeNode& node, const QString& name, double * matrix);


class BRLCADObjectCallback : public BRLCAD::Database::ObjectCallback {
public:

    inline explicit BRLCADObjectCallback(std::function<void(BRLCAD::Object&)> func): func(std::move(func)){}
    inline void operator()(BRLCAD::Object& object) override {
        func(object);
    }

private:
    std::function<void(BRLCAD::Object&)> func;
};

inline void getBRLCADObject(BRLCAD::MemoryDatabase *database, const QString& objectName,const std::function<void(BRLCAD::Object&)>& func){
    BRLCADObjectCallback callback(func);
    database->Get(objectName.toUtf8(), callback);
}


template<typename T>
inline void addPropertiesTitle(T* l, const QString &title, const QString indexText = "") {
    if (title != "") {
        if (!indexText.isEmpty()) {
            QHBoxWidget * hbox = new QHBoxWidget();
            QLabel *padding = new QLabel(indexText);
            QFont font("Monospace");
            font.setStyleHint(QFont::TypeWriter);
            padding->setFont(font);
            padding->setStyleSheet("color:transparent;padding-right:3px;");
            hbox->addWidget(padding);
            QLabel *titleWidget = new QLabel(title);
            titleWidget->setMargin(2);
            hbox->addWidget(titleWidget);
            l->addWidget(hbox);
            hbox->getBoxLayout()->addStretch();
        }
        else{
            QLabel *titleWidget = new QLabel(title);
            titleWidget->setMargin(2);
            l->addWidget(titleWidget);
        }
    }
}

inline QWidget * toolbarSeparator(bool horizontal){
    QWidget *widget = new QWidget;
    if(horizontal){
        widget->setFixedHeight(1);
        widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    }
    else {
        widget->setFixedWidth(1);
        widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    widget->setObjectName("toolbarSeparator");
    return widget;
}

#endif // UTILS_ARBALEST_H