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
#include <QMessageBox>
#include <QHBoxWidget.h>

class Document;
class TestsWidget : public QHBoxWidget {
    Q_OBJECT
public:
    explicit TestsWidget(Document* document, QWidget* parent = nullptr);
    ~TestsWidget();

private:
    Document* document;
    QListWidget* list;
    QTableWidget* table;
    QString dbName;
    QString dbConnectionName;
    
    void dbInit();
    QSqlQuery* dbExec(QString command);
    void popupError(QString message);

    QSqlDatabase getDatabase() const {
        return QSqlDatabase::database(dbConnectionName);
    }
};


#endif // TESTSWIDGET_H