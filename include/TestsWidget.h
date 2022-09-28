//
// Created by isaacy13 on 09/28/2022.
//

#ifndef TESTSWIDGET_H
#define TESTSWIDGET_H

#include <qtreewidget.h>
#include <QTableView>
#include <QSqlTableModel>

class Document;
class TestsWidget : public QWidget {
    Q_OBJECT
public:
    explicit TestsWidget(Document* document);
private:
    Document* document;
    QTableView* resultTable;
    QSqlTableModel* model;
};


#endif // TESTSWIDGET_H