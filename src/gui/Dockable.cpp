
#include <QtWidgets/QLabel>
#include "Dockable.h"

Dockable::Dockable(const QString &dockableTitle, QWidget *mainWindow, QWidget * content) : QDockWidget(dockableTitle, mainWindow) {
    title = new QLabel(dockableTitle);
    title->setObjectName("dockableHeader");
    setTitleBarWidget(title);

    if (content){
        content->setParent(this);
        setWidget(content);
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