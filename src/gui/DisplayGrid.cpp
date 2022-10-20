
#include <QtOpenGL/QtOpenGL>
#include <include/Display.h>
#include "DisplayGrid.h"
#include "MoveCameraMouseAction.h"

DisplayGrid::DisplayGrid(Document*  document) : document(document) {
    verticalSplitter = new QSplitter(this);
    verticalSplitter->setOrientation(Qt::Vertical);
    horizontalSplitter1 = new QSplitter(this);
    horizontalSplitter2 = new QSplitter(this);

    verticalSplitter->addWidget(horizontalSplitter1);
    verticalSplitter->addWidget(horizontalSplitter2);

    displays.append(new Display(document));
    displays.append(new Display(document));
    displays.append(new Display(document));
    displays.append(new Display(document));

    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);

    setMoveCameraMouseAction();

    horizontalSplitter1->addWidget(displays[0]);
    horizontalSplitter1->addWidget(displays[1]);
    horizontalSplitter2->addWidget(displays[2]);
    horizontalSplitter2->addWidget(displays[3]);

    for(int i=0;i<4;i++){
        displays[i]->getCamera()->setAnglesAroundAxes(defaultDisplayCameraRotation[i][0],
                                                      defaultDisplayCameraRotation[i][1],
                                                      defaultDisplayCameraRotation[i][2]);
    }

    addWidget(verticalSplitter);
    activeDisplay = displays[3];

    singleDisplayMode(3);
}

void DisplayGrid::forceRerenderAllDisplays() {
    for (Display *display : displays){
        display->forceRerenderFrame();
    }
}

void DisplayGrid::setActiveDisplay(Display *display) {
    activeDisplay = display;
}

void DisplayGrid::resetViewPort(int displayId) {
    displays[displayId]->getCamera()->setAnglesAroundAxes(defaultDisplayCameraRotation[displayId][0],
                                                          defaultDisplayCameraRotation[displayId][1],
                                                          defaultDisplayCameraRotation[displayId][2]);

    displays[displayId]->getCamera()->autoview();
    displays[displayId]->forceRerenderFrame();
}

void DisplayGrid::resetAllViewPorts() {
    for(int i=0;i<4;i++)resetViewPort(i);
}

void DisplayGrid::singleDisplayMode(int displayId) {
    for(int i=0;i<4;i++){
        if (i != displayId)displays[i]->hide();
    }
    displays[displayId]->show();
    activeDisplay = displays[displayId];
    verticalSplitter->setHandleWidth(0);

    forceRerenderAllDisplays();
}
void DisplayGrid::quadDisplayMode() {
    for(int i=0;i<4;i++){
        displays[i]->show();
    }

    verticalSplitter->setHandleWidth(5);
    verticalSplitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter1->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter2->setSizes(QList<int>({INT_MAX, INT_MAX}));

    forceRerenderAllDisplays();
}

int DisplayGrid::getActiveDisplayId() {
    for(int i=0;i<4;i++)if(displays[i]==activeDisplay)return i;
    return 3;
}

bool DisplayGrid::inQuadDisplayMode() {
    return !displays[0]->isHidden() && !displays[1]->isHidden();
}

void DisplayGrid::setMoveCameraMouseAction() {
    int displaysSize = displays.size();
    for (int index = 0; index < displaysSize; ++index) {
        if (mouseActions[index] != nullptr) {
            delete mouseActions[index];
        }

        mouseActions[index] = new MoveCameraMouseAction(this, displays[index]);
    }
}

