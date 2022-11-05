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
#include <QMessageBox>
#include <chrono>
#include <stack>
#include <iostream>
#include <ged.h>
#include <bu.h>
#include <bu/uuid.h>
#include <bu/file.h>
using namespace std::chrono;
using namespace std;

BRLCAD::Vector3D operator+(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator-(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator/(const BRLCAD::Vector3D& a, const int& b);
double vector3DLength(const BRLCAD::Vector3D& a);

QString breakStringAtCaps(const QString& in);


struct ColorInfo {
    float red, green, blue;
    bool hasColor;

    QColor toQColor() const{
        return QColor(red*255,green*255,blue*255);
    }

    QString toHexString() const{
        return toQColor().name(QColor::HexRgb);
    }
};

const double * getLeafMatrix(BRLCAD::Combination::ConstTreeNode& node, const QString& name);
void setLeafMatrix(BRLCAD::Combination::TreeNode& node, const QString& name, double * matrix);


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

QImage coloredIcon(QString path, QString colorKey = "");

static std::stack <time_point<high_resolution_clock>> times;
static std::stack <QString> timerNames;
inline void ts(QString name = "Timer"){
    times.push(high_resolution_clock::now());
    timerNames.push(name);
}

inline void te(){
    time_point<high_resolution_clock> startTime = times.top();

    milliseconds duration = duration_cast<milliseconds>( high_resolution_clock::now()-startTime);
    cout << timerNames.top().toStdString()<< "  "<<duration_cast<milliseconds>( high_resolution_clock::now()-startTime).count() <<" ms        "
    <<duration_cast<microseconds>( high_resolution_clock::now()-startTime).count()<<" us"<< endl;
}

class Document;
bool getObjectNameFromUser(QWidget* parent, Document& document, QString& name);

void popup(const QString& message);
struct ged* mgedRun(const QString& cmd, const QString& gFilePath);

QString* generateUUID(const QString& filepath);

#endif // UTILS_ARBALEST_H
