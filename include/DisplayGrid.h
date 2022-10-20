
#ifndef RT3_DISPLAYGRID_H
#define RT3_DISPLAYGRID_H


#include <include/QVBoxWidget.h>
#include <include/Document.h>
#include <include/Display.h>
#include <QSplitter>
//#include "MouseAction.h"

class Display;
class MouseAction;


class DisplayGrid : public QVBoxWidget {
public:
    explicit DisplayGrid(Document*  document);

    void forceRerenderAllDisplays();

    Document *getDocument()  {
        return document;
    }

    QVector<Display *> &getDisplays() {
        return displays;
    }

    Display *getActiveDisplay(){
        return activeDisplay;
    }

    bool inQuadDisplayMode();

    int getActiveDisplayId();

    void setActiveDisplay(Display *display);

    void singleDisplayMode(int displayId);
    void quadDisplayMode();

    void resetViewPort(int displayId);
    void resetAllViewPorts();

    void setMoveCameraMouseAction();
private:
    double defaultDisplayCameraRotation[4][3] = {
            {0, 0, 270},
            {270, 0, 180},
            {270, 0, 270},
            {295, 0, 235}
    };
    Document*  document;
    QVector<Display *> displays;
    QVector<MouseAction *> mouseActions;
    Display *activeDisplay;
    QSplitter *verticalSplitter;
    QSplitter *horizontalSplitter1;
    QSplitter *horizontalSplitter2;

};


#endif //RT3_DISPLAYGRID_H
