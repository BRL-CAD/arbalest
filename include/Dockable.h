
#ifndef RT3_DOCKABLE_H
#define RT3_DOCKABLE_H


#include <QtWidgets/QScrollArea>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QLabel>
#include <unordered_map>

/*
 *  A class customized wrapper for QDockWidget to make it scrollable and support default width sizes
 */

class Dockable: public QDockWidget {
public:

    // A wrapper so I can override sizeHint
    class DefaultWidthScrollArea: public QScrollArea{
    public:
        explicit DefaultWidthScrollArea(QWidget *parent,int defaultWidth) : QScrollArea(parent), defaultWidth(defaultWidth) {}
        QSize sizeHint() const override;
    private:
        int defaultWidth;
    };

    Dockable(const QString &dockableTitle, QWidget *mainWindow, bool scrollable, int width = -1);
    void clear();
    void setContent(QWidget *content);
    ~Dockable() override;

private:
    QLabel *title;
    bool scrollable;
    int width;
    std::unordered_map< QWidget*, QScrollArea *> widgetToScrollAreaMap;
};


#endif //RT3_DOCKABLE_H
