#ifndef RELEASEMOUSEACTION_H
#define RELEASEMOUSEACTION_H

#include "MouseAction.h"

class ReleaseMouseAction : public MouseAction
{
public:
    explicit ReleaseMouseAction(QObject* parent = nullptr, Display* watched = nullptr);
    virtual ~ReleaseMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // RELEASEMOUSEACTION_H