#ifndef MOUSEACTION_H
#define MOUSEACTION_H

#include <QObject>

class MouseAction : public QObject
{
    Q_OBJECT
public:
    explicit MouseAction(QObject *parent = nullptr);

signals:

};

#endif // MOUSEACTION_H
