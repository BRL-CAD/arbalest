
#ifndef RT3_DISPLAYGRID_H
#define RT3_DISPLAYGRID_H


#include <include/QVBoxWidget.h>
#include <include/Document.h>
#include <include/ArbDisplay.h>
#include <QSplitter>

class ArbDisplay;
class MouseAction;


class ArbDisplayGrid : public QVBoxWidget {
Q_OBJECT
public:
    explicit ArbDisplayGrid(Document*  document);

    void forceRerenderAllArbDisplays();

    Document *getDocument()  {
        return document;
    }

    QVector<ArbDisplay *> &getArbDisplays() {
        return displays;
    }

    ArbDisplay *getActiveArbDisplay(){
        return activeArbDisplay;
    }

    bool inQuadArbDisplayMode();

    int getActiveArbDisplayId();

    void setActiveArbDisplay(ArbDisplay *display);

    void singleArbDisplayMode(int displayId);
    void quadArbDisplayMode();

    void resetViewPort(int displayId);
    void resetAllViewPorts();

    void setMoveCameraMouseAction();
    void setSelectObjectMouseAction();

private:
    double defaultArbDisplayCameraRotation[4][3] = {
            {0, 0, 270},
            {270, 0, 180},
            {270, 0, 270},
            {295, 0, 235}
    };
    Document*  document;
    QVector<ArbDisplay *> displays;
    QVector<MouseAction *> mouseActions;
    ArbDisplay *activeArbDisplay;
    QSplitter *verticalSplitter;
    QSplitter *horizontalSplitter1;
    QSplitter *horizontalSplitter2;

};


#endif //RT3_DISPLAYGRID_H
