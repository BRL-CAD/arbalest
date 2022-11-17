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
#include <bu/app.h>
#include <QtWidgets>
#include <QMessageBox>
#include <QHBoxWidget.h>
#include "Utils.h"
#include "VerificationValidation.h"
#include "MgedWidget.h"

#define RESULT_CODE_COLUMN 0
#define TEST_NAME_COLUMN 1
#define DESCRIPTION_COLUMN 2
#define OBJPATH_COLUMN 3
#define OBJECT_COLUMN 4
#define TEST_RESULT_ID_COLUMN 5
#define RESULT_TABLE_IDX 6
#define ERROR_TYPE 7

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
    QString getDBConnectionName() const { return dbConnectionName; }

    void showNewTestDialog();
    void showRemoveTestDialog();
    void showNewTestSuiteDialog();
    void showRemoveTestSuiteDialog();

private slots:
	void updateSuiteSelectAll(QListWidgetItem*);
	void updateTestSelectAll(QListWidgetItem*);
	void updateTestListWidget(QListWidgetItem*);
    void testListSelection(QListWidgetItem*);
    void copyToClipboard(QTableWidgetItem*);
    void setupResultMenu(const QPoint&);
    void setupDetailedResult(QTableWidgetItem*);
    void visualizeObjects(QList<QTableWidgetItem*>);
    void searchTests_run(const QString &input);
    void searchTests_rm(const QString &input);
    void searchTests_TS(const QString &input);
    void searchTests_SA();
    void userInputDialogUI(QListWidgetItem*);
    void createTest();
    void removeTests();
    void createSuite();
    void removeSuites();
    void addArgForm();
    void rmvArgForm();
    void isVarClicked(int state);
    void resultTableChangeSize();
    void showResult(const QString& testResultID);

private:
    MainWindow *mainWindow;
    Dockable *parentDockable;
    int msgBoxRes;
    QString cacheFolder;

    // widget-specific data
    Document *document;
    QString modelID;
    QString dbFilePath;
    QString dbConnectionName;

    // user interface data
    QTableWidget* resultTable;
    QListWidget* testList;
    QListWidget* suiteList;
    QListWidget* test_sa;
    QListWidget* suite_sa;
    QDialog* selectTestsDialog;
    QClipboard* clipboard;
    QWidget* content_widget;
    std::vector<QGroupBox*> argForms;

    MgedWidget* terminal;

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
    int resultTableSortIdx;
    std::vector<int> nonResultItemList;
    
    std::map<QListWidgetItem*, std::pair<int, VerificationValidation::Test>> itemToTestMap;
    std::map<int, QListWidgetItem*> idToItemMap;

    // init functions
    void dbConnect(const QString& dbFilePath);
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

    // ui stuff
    void showAllResults();

    // Other
    void checkSuiteSA();
    void checkTestSA();
    QList<QListWidgetItem*> getSelectedTests();
    void validateChecksum();
    void addItemFromTest(QListWidget* &listWidget);
};

#endif // VVWIDGET_H

#ifndef MGED_WORKER_H
#define MGED_WORKER_H
class MgedWorker : public QThread {
    Q_OBJECT
public:
    MgedWorker(const QList<QListWidgetItem*>& selected_tests, const QStringList& selectedObjects, const int& totalTests, const std::map<QListWidgetItem*, std::pair<int, Test>>& itemToTestMap, 
        const QString& modelID, const QString& dbConnectionName, const QString& gFilePath, const QString& dbFilePath) 
        : selected_tests(selected_tests), selectedObjects(selectedObjects), totalTests(totalTests), itemToTestMap(itemToTestMap), 
        modelID(modelID), dbConnectionName(dbConnectionName + "_run_tests"), gFilePath(gFilePath), dbFilePath(dbFilePath)
    {
        std::cout << "dbconnname: " << this->dbConnectionName.toStdString() << std::endl;
        std::cout << "dbfilepath: " << this->dbFilePath.toStdString() << std::endl;
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", this->dbConnectionName);
        db.setDatabaseName(this->dbFilePath);

        if (!db.open() || !db.isOpen())
            throw std::runtime_error("[MgedWorker] ERROR: db failed to open: " + db.lastError().text().toStdString());
    }
    QSqlDatabase getDatabase() const { return QSqlDatabase::database(this->dbConnectionName, false); }
    void run() override;

signals:
    void updateStatusBarRequest(bool testRan, int currTest, int totalTests, int currObject, int totalObjects);
    void showResultRequest(const QString& testResultID);

private:
    const QList<QListWidgetItem*> selected_tests;
    const std::map<QListWidgetItem*, std::pair<int, Test>> itemToTestMap;
    const QStringList selectedObjects;
    
    const QString modelID;
    const QString dbConnectionName;
    const QString gFilePath;
    const QString dbFilePath;
    const int totalTests;
};
#endif