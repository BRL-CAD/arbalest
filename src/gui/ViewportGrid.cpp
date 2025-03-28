
#include <QtOpenGL/QtOpenGL>
#include <include/Viewport.h>
#include "ViewportGrid.h"
#include "MoveCameraMouseAction.h"
#include "SelectMouseAction.h"

ViewportGrid::ViewportGrid(Document*  document) : document(document) {
    verticalSplitter = new QSplitter(this);
    verticalSplitter->setOrientation(Qt::Vertical);
    horizontalSplitter1 = new QSplitter(this);
    horizontalSplitter2 = new QSplitter(this);

    verticalSplitter->addWidget(horizontalSplitter1);
    verticalSplitter->addWidget(horizontalSplitter2);

    displays.append(new Viewport(document));
    displays.append(new Viewport(document));
    displays.append(new Viewport(document));
    displays.append(new Viewport(document));

    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);

    horizontalSplitter1->addWidget(displays[0]);
    horizontalSplitter1->addWidget(displays[1]);
    horizontalSplitter2->addWidget(displays[2]);
    horizontalSplitter2->addWidget(displays[3]);

    for(int i=0;i<4;i++){
        displays[i]->getCamera()->setAnglesAroundAxes(defaultViewportCameraRotation[i][0],
                                                      defaultViewportCameraRotation[i][1],
                                                      defaultViewportCameraRotation[i][2]);
    }

    addWidget(verticalSplitter);
    activeViewport = displays[3];

    singleViewportMode(3);
}

void ViewportGrid::forceRerenderAllViewports() {
    for (Viewport *display : displays){
        display->forceRerenderFrame();
    }
}

void ViewportGrid::setActiveViewport(Viewport *display) {
    activeViewport = display;
}

void ViewportGrid::resetViewPort(int displayId) {
    displays[displayId]->getCamera()->setAnglesAroundAxes(defaultViewportCameraRotation[displayId][0],
                                                          defaultViewportCameraRotation[displayId][1],
                                                          defaultViewportCameraRotation[displayId][2]);

    displays[displayId]->getCamera()->autoview();
    displays[displayId]->forceRerenderFrame();
}

void ViewportGrid::resetAllViewPorts() {
    for(int i=0;i<4;i++)resetViewPort(i);
}

void ViewportGrid::singleViewportMode(int displayId) {
    for(int i=0;i<4;i++){
        if (i != displayId)displays[i]->hide();
    }
    displays[displayId]->show();
    activeViewport = displays[displayId];
    verticalSplitter->setHandleWidth(0);

    forceRerenderAllViewports();
}
void ViewportGrid::quadViewportMode() {
    for(int i=0;i<4;i++){
        displays[i]->show();
    }

    verticalSplitter->setHandleWidth(5);
    verticalSplitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter1->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter2->setSizes(QList<int>({INT_MAX, INT_MAX}));

    forceRerenderAllViewports();
}

int ViewportGrid::getActiveViewportId() {
    for(int i=0;i<4;i++)if(displays[i]==activeViewport)return i;
    return 3;
}

bool ViewportGrid::inQuadViewportMode() {
    return !displays[0]->isHidden() && !displays[1]->isHidden();
}

void ViewportGrid::setMoveCameraMouseAction() {
    int displaysSize = displays.size();
    for (int index = 0; index < displaysSize; ++index) {
        if (mouseActions[index] != nullptr) {
            delete mouseActions[index];
        }

        mouseActions[index] = new MoveCameraMouseAction(this, displays[index]);
    }
}

void ViewportGrid::setSelectObjectMouseAction() {
    int displaysSize = displays.size();
    for (int index = 0; index < displaysSize; ++index) {
        if (mouseActions[index] != nullptr) {
            delete mouseActions[index];
        }

        mouseActions[index] = new SelectMouseAction(this, displays[index]);
        connect(mouseActions[index], &MouseAction::Done, this, [this](MouseAction* mouseAction) {
            SelectMouseAction* selectMouseAction = dynamic_cast<SelectMouseAction*>(mouseAction);

            if (selectMouseAction != nullptr) {
                QString regionName = selectMouseAction->getSelected();
                document->getObjectTreeWidget()->select(regionName);
            }
        });
    }
}
