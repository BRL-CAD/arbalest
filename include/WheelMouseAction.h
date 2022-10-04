#ifndef WHEELMOUSEACTION_H
#define WHEELMOUSEACTION_H

#include "MouseAction.h"

class WheelMouseAction : public MouseAction
{
public:
    explicit WheelMouseAction(QObject* parent = nullptr, Display* watched = nullptr);
    virtual ~WheelMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // WHEELMOUSEACTION_H