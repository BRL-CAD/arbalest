#ifndef MOVECAMERAMOUSEACTION_H
#define MOVECAMERAMOUSEACTION_H

#include "MouseAction.h"

class MoveCameraMouseAction : public MouseAction
{
public:
    explicit MoveCameraMouseAction(QObject* parent = nullptr, Display* watched = nullptr);
    virtual ~MoveCameraMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    int prevMouseX = -1;
    int prevMouseY = -1;
    bool skipNextMouseMoveEvent = false;

    Qt::MouseButton rotateCameraMouseButton = Qt::LeftButton;
    Qt::MouseButton moveCameraMouseButton = Qt::RightButton;
    Qt::KeyboardModifier rotateAroundThirdAxisModifier = Qt::ShiftModifier;
};

#endif // MOVECAMERAMOUSEACTION_H