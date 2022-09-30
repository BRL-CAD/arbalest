#ifndef MOUSEACTION_H
#define MOUSEACTION_H

#include <QObject>
#include <QEvent>
#include "Display.h"

class MouseAction : public QObject
{
    Q_OBJECT
public:
    virtual ~MouseAction();

protected:
    QObject* m_watched;

    explicit MouseAction(QObject* parent = nullptr, Display* watched = nullptr);
    virtual bool eventFilter(QObject* watched, QEvent* event) = 0;

signals:
    void Done(MouseAction* myself);

private slots:
    void WatchedDestroyed(QObject* watched);
};

#endif // MOUSEACTION_H