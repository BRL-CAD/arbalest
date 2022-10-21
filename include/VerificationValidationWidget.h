//
// Created by isaacy13 on 09/28/2022.
//

#ifndef VVWIDGET_H
#define VVWIDGET_H

#include <ged.h>
#include <bu.h>
#include <iostream>
#include <QTableView>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtWidgets>
#include <QMessageBox>
#include <QHBoxWidget.h>
#include "VerificationValidation.h"
#include "Utils.h"

class Document;
class VerificationValidationWidget : public QHBoxWidget {
    Q_OBJECT
public:
    explicit VerificationValidationWidget(Document* document, QWidget* parent = nullptr);
    ~VerificationValidationWidget();
    void showSelectTests();
    QString* runTest(const QString& cmd);
    void runTests();
    void setStatusBar(QStatusBar* statusBar) { this->statusBar = statusBar; }

private:
    // widget-specific data
    Document* document;
    QString modelID;
    QString dbName;
    QString dbConnectionName;

    // user interface data
    QTableWidget* resultTable;
    QListWidget* testList;
    QDialog* selectTestsDialog;
    QStatusBar* statusBar;

    // init functions
    void dbConnect();
    void dbInitTables();
    void dbPopulateDefaults();
    void setupUI();

    // database functions
    QSqlQuery* dbExec(QString command, bool showErrorPopup = true);
    void dbExec(QSqlQuery*& query, bool showErrorPopup = true);
    QSqlDatabase getDatabase() const {
        return QSqlDatabase::database(dbConnectionName);
    }

    // events
    void resizeEvent(QResizeEvent* event) override;
};

#endif // VVWIDGET_H