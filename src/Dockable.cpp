
#include <QtWidgets/QLabel>
#include "Dockable.h"

Dockable::Dockable(const QString &title, QWidget *mainWindow, QWidget * content) : QDockWidget(title, mainWindow) {
    titleLabel = new QLabel(title);
    titleLabel->setObjectName("dockableHeader");
    setTitleBarWidget(titleLabel);

    if (content){
        content->setParent(this);
        setWidget(content);
    }
}

Dockable::Dockable(const QString &title, QWidget *mainWindow, Dockable::FillerObject fillerObject):
                                                                        Dockable(title,mainWindow,nullptr){
    fillWithPlaceholder(fillerObject);
}

void Dockable::fillWithPlaceholder( Dockable::FillerObject fillerObject){
    QWidget * placeholder = new QWidget(this);
    if (fillerObject == WideFillerObject) placeholder->setObjectName("dockableContentWide");
    else placeholder->setObjectName("dockableContent");
    setWidget(placeholder);
}