//
// Created by isaacy13 on 09/28/2022.
//

#ifndef VVWIDGET_H
#define VVWIDGET_H

#include <iostream>
#include <QTableView>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtWidgets>
#include <QMessageBox>
#include <QHBoxWidget.h>

class Document;
class VerificationValidationWidget : public QHBoxWidget {
    Q_OBJECT
public:
    explicit VerificationValidationWidget(Document* document, QWidget* parent = nullptr);
    ~VerificationValidationWidget();

private:
    // widget-specific data
    Document* document;
    QString modelID;
    QString dbName;
    QString dbConnectionName;
    
    // user interface
    QListWidget* testList;
    QTableWidget* resultTable;

    // functions
    void dbConnect();
    void dbInitTables();
    void dbPopulateTables();
    void dbInitDummyData();
    void setupUI();
    QSqlQuery* dbExec(QString command, bool showErrorPopup = true);
    void popup(QString message);

    QSqlDatabase getDatabase() const {
        return QSqlDatabase::database(dbConnectionName);
    }
};


#endif // VVWIDGET_H