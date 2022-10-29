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

#define RESULT_CODE_COLUMN 0
#define TEST_NAME_COLUMN 1
#define DESCRIPTION_COLUMN 2
#define OBJPATH_COLUMN 3

#define NO_SELECTION -1
#define OPEN 0
#define DISCARD 1
#define CANCEL 2

class MainWindow;
class Document;
class Dockable;
class VerificationValidationWidget : public QHBoxWidget
{
    Q_OBJECT
public:
    explicit VerificationValidationWidget(MainWindow *mainWindow, Document *document, QWidget *parent = nullptr);
    ~VerificationValidationWidget();
    void showSelectTests();
    QString *runTest(const QString &cmd);
    void loadATRFile(const QString &filepath);
    void setStatusBar(QStatusBar *statusBar) { this->statusBar = statusBar; }

    QString getDBConnectionName() const
    {
        return dbConnectionName;
    }

public slots:
    void runTests();

private slots:
	void updateSuiteSelectAll(QListWidgetItem*);
	void updateTestSelectAll(QListWidgetItem*);
	void updateTestListWidget(QListWidgetItem*);
    void testListSelection(QListWidgetItem*);
    void setupDetailedResult(int row, int  column);
    void searchTests(const QString &input);
    void userInputDialogUI(QListWidgetItem*);

private:
    MainWindow *mainWindow;
    Dockable *parentDockable;
    int msgBoxRes;

    // widget-specific data
    Document *document;
    QString modelID;
    QString dbName;
    QString dbConnectionName;

    // user interface data
    QTableWidget* resultTable;
    QListWidget* testList;
    QListWidget* suiteList;
    QListWidget* test_sa;
    QListWidget* suite_sa;
    QLineEdit* searchBox;
    QDialog* selectTestsDialog;
    QStatusBar* statusBar;

    // init functions
    void dbConnect(QString dbName);
    void dbInitTables();
    void dbPopulateDefaults();
    void setupUI();

    // database functions
    QSqlQuery *dbExec(QString command, bool showErrorPopup = true);
    void dbExec(QSqlQuery *&query, bool showErrorPopup = true);
    QSqlDatabase getDatabase() const
    {
        return QSqlDatabase::database(dbConnectionName);
    }

    void dbClearResults();

    // events
    void resizeEvent(QResizeEvent *event) override;

    // ui stuff
    void showResult(const QString &testResultID);
    void showAllResults();

    // Other
    void checkSuiteSA();
    void checkTestSA();
};

#endif // VVWIDGET_H
