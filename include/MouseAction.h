#ifndef MOUSEACTION_H
#define MOUSEACTION_H

#include <QObject>

class MouseAction : public QObject
{
    Q_OBJECT
public:
    virtual ~MouseAction();

signals:
    void Done(MouseAction* myself);

protected:
    QObject* m_watched;

    MouseAction(QObject* watched);
    virtual bool eventFilter(QObject* watched, QEvent* event) = 0;

private slots:
    void WatchedDestroyed(QObject* watched);
};

#endif // MOUSEACTION_H
