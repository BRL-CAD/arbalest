
#ifndef RT3_SUBWINDOW_H
#define RT3_SUBWINDOW_H


#include <QtWidgets/QMdiSubWindow>

class SubWindow: public QMdiSubWindow{
public:
    explicit SubWindow(const int documentId, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags()) : QMdiSubWindow(parent, flags),
                                                                                                                     documentId(documentId) {}

    const int getDocumentId() const {
        return documentId;
    }

private:
    const int documentId;
};


#endif //RT3_SUBWINDOW_H
