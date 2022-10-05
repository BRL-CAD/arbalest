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
};

#endif // MOVECAMERAMOUSEACTION_H