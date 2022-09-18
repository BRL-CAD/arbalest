#ifndef DRAGWINDOWMOUSEACTION_H
#define DRAGWINDOWMOUSEACTION_H

#include "MouseAction.h"
#include <QMainWindow>
#include <QMenuBar>

class DragWindowMouseAction : public MouseAction
{
public:
    DragWindowMouseAction(QMainWindow *mainWindow, QMenuBar* menuTitleBar);
    virtual ~DragWindowMouseAction();

protected:
    QMainWindow* m_mainWindow;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // DRAGWINDOWMOUSEACTION_H
