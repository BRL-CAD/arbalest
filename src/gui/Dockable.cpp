
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include "Dockable.h"

Dockable::Dockable(const QString &dockableTitle, QWidget *mainWindow, QWidget * content, bool scrollable) : QDockWidget(dockableTitle, mainWindow) {
    title = new QLabel(dockableTitle);
    title->setObjectName("dockableHeader");
    setTitleBarWidget(title);

    if (content){
        content->setParent(this);
        if (!scrollable)setWidget(content);
        else{
            QScrollArea* scrollArea = new QScrollArea(this);
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(content);
            scrollArea->setObjectName("dockableContent");
            setWidget(scrollArea);
        }
    }
}

Dockable::Dockable(const QString &dockableTitle, QWidget *mainWindow, Dockable::FillerObject fillerObject):
                                                                        Dockable(dockableTitle, mainWindow){
    fillWithPlaceholder(fillerObject);
}

void Dockable::fillWithPlaceholder( Dockable::FillerObject fillerObject){
    QWidget * placeholder = new QWidget(this);
    if (fillerObject == WideFillerObject) placeholder->setObjectName("dockableContentWide");
    else placeholder->setObjectName("dockableContent");
    setWidget(placeholder);
}