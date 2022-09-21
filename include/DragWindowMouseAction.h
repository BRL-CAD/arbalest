#ifndef DRAGWINDOWMOUSEACTION_H
#define DRAGWINDOWMOUSEACTION_H

#include "MouseAction.h"
#include <QMainWindow>
#include <QMenuBar>

class DragWindowMouseAction : public MouseAction
{
public:
    DragWindowMouseAction(QObject *watched, QWidget* widget);
    virtual ~DragWindowMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // DRAGWINDOWMOUSEACTION_H
