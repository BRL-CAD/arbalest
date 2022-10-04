#ifndef PRESSMOUSEACTION_H
#define PRESSMOUSEACTION_H

#include "MouseAction.h"

class PressMouseAction : public MouseAction
{
public:
    explicit PressMouseAction(QObject* parent = nullptr, Display* watched = nullptr);
    virtual ~PressMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // PRESSMOUSEACTION_H