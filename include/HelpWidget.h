
#ifndef RT3_helpWINDOW_H
#define RT3_helpWINDOW_H


#include "QVBoxWidget.h"

class HelpWidget : public QVBoxWidget{
    Q_OBJECT
public:
    HelpWidget();
private slots:
    void onLinkClicked(const QString& link);
signals:
    void ctrlNPressed();
    void ctrlOPressed();
};


#endif //RT3_helpWINDOW_H
