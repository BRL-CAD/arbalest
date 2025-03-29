
#ifndef RT3_VIEWPORTGRID_H
#define RT3_VIEWPORTGRID_H


#include <include/QVBoxWidget.h>
#include <include/Document.h>
#include <include/Viewport.h>
#include <QSplitter>

class Viewport;
class MouseAction;


class ViewportGrid : public QVBoxWidget {
Q_OBJECT
public:
    explicit ViewportGrid(Document*  document);

    void forceRerenderAllViewports();

    Document *getDocument()  {
        return document;
    }

    QVector<Viewport *> &getViewports() {
        return displays;
    }

    Viewport *getActiveViewport(){
        return activeViewport;
    }

    bool inQuadViewportMode();

    int getActiveViewportId();

    void setActiveViewport(Viewport *display);

    void singleViewportMode(int displayId);
    void quadViewportMode();

    void resetViewPort(int displayId);
    void resetAllViewPorts();

    void setMoveCameraMouseAction();
    void setSelectObjectMouseAction();

private:
    double defaultViewportCameraRotation[4][3] = {
            {0, 0, 270},
            {270, 0, 180},
            {270, 0, 270},
            {295, 0, 235}
    };
    Document*  document;
    QVector<Viewport *> displays;
    QVector<MouseAction *> mouseActions;
    Viewport *activeViewport;
    QSplitter *verticalSplitter;
    QSplitter *horizontalSplitter1;
    QSplitter *horizontalSplitter2;

};


#endif //RT3_VIEWPORTGRID_H
