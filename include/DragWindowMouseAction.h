#ifndef DRAGWINDOWMOUSEACTION_H
#define DRAGWINDOWMOUSEACTION_H

#include "MouseAction.h"
#include <QMainWindow>
#include <QMenuBar>

class DragWindowMouseAction : public MouseAction
{
public:
    DragWindowMouseAction(QWidget* widget, QWidget* mainWindow, QObject* watched = nullptr);
    virtual ~DragWindowMouseAction();

protected:
    QWidget* m_mainWindow;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // DRAGWINDOWMOUSEACTION_H
