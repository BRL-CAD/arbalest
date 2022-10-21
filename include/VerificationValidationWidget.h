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
#include "VerificationValidationParser.h"
#include "Utils.h"

#ifndef VVTEST_H
#define VVTEST_H
class VerificationValidationTest {
public:
    QString testName;
    QString testCommand;
    QString suiteName;
};
#endif

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

    class DefaultTests {
        public:
        const static VerificationValidationTest MISMATCHED_DUP_IDS;
        const static VerificationValidationTest NO_NESTED_REGIONS;
        const static VerificationValidationTest NO_EMPTY_COMBOS;
        const static VerificationValidationTest NO_SOLIDS_OUTSIDE_REGIONS;
        const static VerificationValidationTest ALL_BOTS_VOLUME_MODE;
        const static VerificationValidationTest NO_BOTS_LH_ORIENT; // TODO: this command can run faster if use unix
        const static VerificationValidationTest ALL_REGIONS_MAT;
        const static VerificationValidationTest ALL_REGIONS_LOS;
        const static VerificationValidationTest NO_NULL_REGIONS;
        const static VerificationValidationTest NO_OVERLAPS;
        const static VerificationValidationTest NO_DUPLICATE_ID;
        const static VerificationValidationTest NO_MATRICES;
        const static VerificationValidationTest NO_INVALID_AIRCODE_REGIONS;
        const static std::vector<VerificationValidationTest> allTests;
        // TODO: missing "No errors when top level drawn"
        // TODO: missing "BoTs are valid"
        // TODO: missing "Air does not stick out"
        // TODO: missing "Title"
        // TODO: missing "Ground plane at z=0"
    };

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