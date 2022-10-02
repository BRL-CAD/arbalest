//
// Created by isaacy13 on 09/28/2022.
//

#ifndef TESTSWIDGET_H
#define TESTSWIDGET_H

#include <iostream>
#include <QTableView>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtWidgets>
#include <QHBoxWidget.h>

class Document;
class TestsWidget : public QHBoxWidget {
    Q_OBJECT
public:
    explicit TestsWidget(Document* document, QWidget* parent = nullptr);

private:
    Document* document;
    QListWidget* list;
    QTableWidget* table;
    QSqlDatabase db;
    void dbInit();
    QSqlQuery* dbExec(QString command);
};


#endif // TESTSWIDGET_H