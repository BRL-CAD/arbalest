
#ifndef RT3_DOCKABLE_H
#define RT3_DOCKABLE_H


#include <QtWidgets/QScrollArea>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QLabel>
#include <unordered_map>

class Dockable: public QDockWidget {
public:
    Dockable(const QString &dockableTitle, QWidget *mainWindow, bool scrollable, bool wide);
    void clear();
    void setContent(QWidget *content);
    virtual ~Dockable() = default;
private:
    QLabel *title;
    QWidget * filler;
    bool scrollable;
    bool wide;
    std::unordered_map< QWidget*, QScrollArea *> widgetToScrollAreaMap;
};


#endif //RT3_DOCKABLE_H
