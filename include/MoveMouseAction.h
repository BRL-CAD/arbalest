#ifndef MOVEMOUSEACTION_H
#define MOVEMOUSEACTION_H

#include <QObject>
#include "MouseAction.h"

class MoveMouseAction : public MouseAction
{
    Q_OBJECT
public:
    explicit MoveMouseAction(QObject* parent = nullptr, Display* watched = nullptr);
    virtual ~MoveMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) = 0;
};

#endif // MOVEMOUSEACTION_H