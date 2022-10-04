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


#endif // VVWIDGET_H