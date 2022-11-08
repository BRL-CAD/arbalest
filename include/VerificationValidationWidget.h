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
#include "Utils.h"
#include "VerificationValidation.h"

#define RESULT_CODE_COLUMN 0
#define TEST_NAME_COLUMN 1
#define DESCRIPTION_COLUMN 2
#define OBJPATH_COLUMN 3
#define OBJECT_COLUMN 4
#define TEST_RESULT_ID_COLUMN 5

#define NO_SELECTION -1
#define OPEN 0
#define DISCARD 1
#define CANCEL 2

class MainWindow;
class Document;
class Dockable;
using Arg = VerificationValidation::Arg;
using Test = VerificationValidation::Test;

class VerificationValidationWidget : public QHBoxWidget
{
    Q_OBJECT
public:
    explicit VerificationValidationWidget(MainWindow *mainWindow, Document *document, QWidget *parent = nullptr);
    ~VerificationValidationWidget();
    void updateDockableHeader();
    void showSelectTests();
    void setStatusBar(QStatusBar* statusBar) { this->statusBar = statusBar; }
    QString getDBConnectionName() const { return dbConnectionName; }

    void showNewTestDialog();
    void showRemoveTestDialog();
    void showNewTestSuiteDialog();
    void showRemoveTestSuiteDialog();
public slots:
    void runTests();

private slots:
	void updateSuiteSelectAll(QListWidgetItem*);
	void updateTestSelectAll(QListWidgetItem*);
	void updateTestListWidget(QListWidgetItem*);
    void testListSelection(QListWidgetItem*);
    void copyToClipboard();
    void setupResultMenu(int row, int column);
    void setupDetailedResult();
    void visualizeOverlaps();
    void searchTests_run(const QString &input);
    void searchTests_rm(const QString &input);
    void searchTests_TS(const QString &input);
    void userInputDialogUI(QListWidgetItem*);
    void createTest();
    void removeTests();
    void createSuite();
    void removeSuites();
    void addArgForm();
    void rmvArgForm();
    void isVarClicked(int state);
    void resultTableChangeSize();

private:
    MainWindow *mainWindow;
    Dockable *parentDockable;
    int msgBoxRes;
    QString folderName;

    // widget-specific data
    Document *document;
    QString modelID;
    QString dbFilePath;
    QString dbName;
    QString dbConnectionName;

    // user interface data
    QTableWidget* resultTable;
    QListWidget* testList;
    QListWidget* suiteList;
    QListWidget* test_sa;
    QListWidget* suite_sa;
    QDialog* selectTestsDialog;
    QStatusBar* statusBar;
    QClipboard* clipboard;
    int currentResultRow = 1;
    QWidget* content_widget;
    std::vector<QGroupBox*> argForms;

    // Test and test suite create remove
    QLineEdit* testNameInput;
    QLineEdit* testCmdInput;
    QLineEdit* testCategoryInput;
    QListWidget* addToSuiteList;
    QHBoxLayout* argLayout;
    std::vector<QLineEdit*> argInputList;
    std::vector<QCheckBox*> isVarList;
    std::vector<QLineEdit*> varInputList;

    QListWidget* rmTestList;
    QListWidget* newTSList;
    QListWidget* rmTSList;
    QLineEdit* suiteNameBox;
    bool minBtn_toggle;
    QToolButton* minBtn;
    
    std::map<QListWidgetItem*, std::pair<int, VerificationValidation::Test>> itemToTestMap;
    std::map<int, QListWidgetItem*> idToItemMap;

    // init functions
    void dbConnect(QString dbFilePath);
    void dbInitTables();
    void dbPopulateDefaults();
    void setupUI();

    // database functions
    QSqlQuery *dbExec(QString command, bool showErrorPopup = true);
    void dbExec(QSqlQuery *&query, bool showErrorPopup = true);
    void dbClose() {
        { 
            QSqlDatabase db = getDatabase();
            if (db.isOpen()) db.close();
        }
        QSqlDatabase::removeDatabase(dbConnectionName);
    }

    QSqlDatabase getDatabase() const
    {
        return QSqlDatabase::database(dbConnectionName, false);
    }

    bool dbIsAlive(QSqlDatabase db) {
        if (!db.isOpen()) return false;
        QSqlQuery q("SELECT 1 FROM Tests", db);
        if (!q.isActive()) return false;
        return true;
    }

    void dbUpdateModelUUID();
    void dbClearResults();

    // events
    void resizeEvent(QResizeEvent *event) override;

    // ui stuff
    void showResult(const QString &testResultID);
    void showAllResults();

    // Other
    void checkSuiteSA();
    void checkTestSA();
    QString *runTest(const QString &cmd);
    void validateChecksum();
    void addItemFromTest(QListWidget* &listWidget);
};

#endif // VVWIDGET_H
