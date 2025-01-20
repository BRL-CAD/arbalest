
#include <QtOpenGL/QtOpenGL>
#include <include/ArbDisplay.h>
#include "ArbDisplayGrid.h"
#include "MoveCameraMouseAction.h"
#include "SelectMouseAction.h"

ArbDisplayGrid::ArbDisplayGrid(Document*  document) : document(document) {
    verticalSplitter = new QSplitter(this);
    verticalSplitter->setOrientation(Qt::Vertical);
    horizontalSplitter1 = new QSplitter(this);
    horizontalSplitter2 = new QSplitter(this);

    verticalSplitter->addWidget(horizontalSplitter1);
    verticalSplitter->addWidget(horizontalSplitter2);

    displays.append(new ArbDisplay(document));
    displays.append(new ArbDisplay(document));
    displays.append(new ArbDisplay(document));
    displays.append(new ArbDisplay(document));

    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);

    horizontalSplitter1->addWidget(displays[0]);
    horizontalSplitter1->addWidget(displays[1]);
    horizontalSplitter2->addWidget(displays[2]);
    horizontalSplitter2->addWidget(displays[3]);

    for(int i=0;i<4;i++){
        displays[i]->getCamera()->setAnglesAroundAxes(defaultArbDisplayCameraRotation[i][0],
                                                      defaultArbDisplayCameraRotation[i][1],
                                                      defaultArbDisplayCameraRotation[i][2]);
    }

    addWidget(verticalSplitter);
    activeArbDisplay = displays[3];

    singleArbDisplayMode(3);
}

void ArbDisplayGrid::forceRerenderAllArbDisplays() {
    for (ArbDisplay *display : displays){
        display->forceRerenderFrame();
    }
}

void ArbDisplayGrid::setActiveArbDisplay(ArbDisplay *display) {
    activeArbDisplay = display;
}

void ArbDisplayGrid::resetViewPort(int displayId) {
    displays[displayId]->getCamera()->setAnglesAroundAxes(defaultArbDisplayCameraRotation[displayId][0],
                                                          defaultArbDisplayCameraRotation[displayId][1],
                                                          defaultArbDisplayCameraRotation[displayId][2]);

    displays[displayId]->getCamera()->autoview();
    displays[displayId]->forceRerenderFrame();
}

void ArbDisplayGrid::resetAllViewPorts() {
    for(int i=0;i<4;i++)resetViewPort(i);
}

void ArbDisplayGrid::singleArbDisplayMode(int displayId) {
    for(int i=0;i<4;i++){
        if (i != displayId)displays[i]->hide();
    }
    displays[displayId]->show();
    activeArbDisplay = displays[displayId];
    verticalSplitter->setHandleWidth(0);

    forceRerenderAllArbDisplays();
}
void ArbDisplayGrid::quadArbDisplayMode() {
    for(int i=0;i<4;i++){
        displays[i]->show();
    }

    verticalSplitter->setHandleWidth(5);
    verticalSplitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter1->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter2->setSizes(QList<int>({INT_MAX, INT_MAX}));

    forceRerenderAllArbDisplays();
}

int ArbDisplayGrid::getActiveArbDisplayId() {
    for(int i=0;i<4;i++)if(displays[i]==activeArbDisplay)return i;
    return 3;
}

bool ArbDisplayGrid::inQuadArbDisplayMode() {
    return !displays[0]->isHidden() && !displays[1]->isHidden();
}

void ArbDisplayGrid::setMoveCameraMouseAction() {
    int displaysSize = displays.size();
    for (int index = 0; index < displaysSize; ++index) {
        if (mouseActions[index] != nullptr) {
            delete mouseActions[index];
        }

        mouseActions[index] = new MoveCameraMouseAction(this, displays[index]);
    }
}

void ArbDisplayGrid::setSelectObjectMouseAction() {
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
