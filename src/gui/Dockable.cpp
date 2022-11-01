
#include <QtWidgets/QLabel>
#include "Dockable.h"
#include <iostream>

QSize Dockable::DefaultWidthScrollArea::sizeHint() const  {
    QSize hint = QScrollArea::sizeHint();
    if(defaultWidth != -1) hint.setWidth(defaultWidth);
    return hint;
}

Dockable::Dockable(const QString &dockableTitle, QWidget *mainWindow, bool scrollable, int width) :
        scrollable(scrollable),
        width(width),
        QDockWidget(dockableTitle, mainWindow) {
    title = new QLabel(dockableTitle);
    title->setObjectName("dockableHeader");
    setTitleBarWidget(title);
    fillerWidget.width = width;
    fillerWidget.setObjectName("dockableContent");
    clear();
}

void Dockable::setContent(QWidget *content) {
    if (!content) return;
    if(widget())widget()->setVisible(false);
    if (scrollable) { // use scroll areas
        QScrollArea *scrollArea;
        if (widgetToScrollAreaMap.find(content) == widgetToScrollAreaMap.end()) {
            scrollArea = new DefaultWidthScrollArea(this,width);
            widgetToScrollAreaMap[content] = scrollArea;
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(content);
        } else {
            scrollArea = widgetToScrollAreaMap[content];
        }
        setWidget(scrollArea);
        if (!scrollArea->isVisible()) scrollArea->setVisible(true);
    } else {    // no scroll areas
        setWidget(content);
    }
    if (!content->isVisible()) content->setVisible(true);
}

void Dockable::hideHeader()
{
    setTitleBarWidget(new QWidget(this));
}

void Dockable::clear() {
    if(widget()){
        widget()->setVisible(false);
        widget()->setParent(nullptr);
    }
    setContent(&fillerWidget);
}

Dockable::~Dockable() {
    widget()->setParent(nullptr);
    for (std::pair< QWidget*, QScrollArea *> i : widgetToScrollAreaMap){
        QScrollArea * scrollArea = i.second;
        if(scrollArea->widget() && scrollArea->widget()->parent() == scrollArea){ // break content objects' child parent for children to survive
            scrollArea->widget()->setVisible(false);
            scrollArea->widget()->setParent(nullptr);
            delete scrollArea;
        }
    }
}
