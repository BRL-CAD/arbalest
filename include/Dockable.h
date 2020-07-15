
#ifndef RT3_DOCKABLE_H
#define RT3_DOCKABLE_H


#include <QtWidgets/QDockWidget>
#include <QtWidgets/QLabel>

class Dockable: public QDockWidget {
public:
    enum FillerObject {SmallFillerObject, WideFillerObject};

    Dockable(const QString &dockableTitle, QWidget *mainWindow, QWidget * content = nullptr);
    Dockable(const QString &dockableTitle, QWidget *mainWindow, enum FillerObject fillerObject);
    void fillWithPlaceholder(FillerObject fillerObject);
    virtual ~Dockable() = default;
private:
    QLabel *title;

};


#endif //RT3_DOCKABLE_H
