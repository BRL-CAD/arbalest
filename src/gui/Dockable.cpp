
#include <QtWidgets/QLabel>
#include "Dockable.h"

Dockable::Dockable(const QString &dockableTitle, QWidget *mainWindow, bool scrollable, bool wide) :
        scrollable(scrollable),
        wide(wide),
        QDockWidget(dockableTitle, mainWindow) {

    title = new QLabel(dockableTitle);
    title->setObjectName("dockableHeader");
    setTitleBarWidget(title);

    filler = new QWidget(this);
    if (wide)
        filler->setObjectName("dockableContentWide");
    else
        filler->setObjectName("dockableContent");
    clear();
}

void Dockable::setContent(QWidget *content) {
    if (content != filler && scrollable) { // use scroll areas
        QScrollArea *scrollArea;
        if (widgetToScrollAreaMap.find(content) == widgetToScrollAreaMap.end()) {
            scrollArea = new QScrollArea(this);
            widgetToScrollAreaMap[content] = scrollArea;
            scrollArea->setWidgetResizable(true);
            scrollArea->setObjectName(wide ? "dockableContentWide" : "dockableContent");
            scrollArea->setWidget(content);
        } else {
            scrollArea = widgetToScrollAreaMap[content];
        }
        setWidget(scrollArea);
    } else {    // no scroll areas
        setWidget(content);
    }
}

void Dockable::clear() {
    setWidget(filler);
}