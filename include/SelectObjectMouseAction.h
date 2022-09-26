#ifndef SELECTOBJECTMOUSEACTION_H
#define SELECTOBJECTMOUSEACTION_H

#include "MouseAction.h"

class SelectObjectMouseAction : public MouseAction
{
public:
    SelectObjectMouseAction(QObject* watched = nullptr);
    virtual ~SelectObjectMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // SELECTOBJECTMOUSEACTION_H
