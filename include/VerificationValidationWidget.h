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
#include "Utils.h"

struct VerificationValidationTest {
    QString testName;
    QString testCommand;
    QString suiteName;
};

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
    QSqlQuery* dbExec(QSqlQuery* query, bool showErrorPopup = true);
    QSqlDatabase getDatabase() const {
        return QSqlDatabase::database(dbConnectionName);
    }

    // events
    void resizeEvent(QResizeEvent* event) override;

    // constant defaults
    const std::vector<VerificationValidationTest> defaultTests = {
        {"No mis-matched dupliate IDs", "lc -m all", "General"},
        {"No nested regions", "search /all -type region -below -type region", "General"},
        {"No empty combos", "search /all -nnodes 0", "General"},
        {"No solids outside of regions", "search /all ! -below -type region -type shape", "General"},
        {"All BoTs are volume mode (should return nothing)", "search all -type bot ! -type volume", "General"},
        {"No BoTs are left hand orientation", "search all -type bot -param orient=lh", "General"}, // TODO: this command can run faster if use unix
        {"All regions have material & LOS", "search /all -type region ! -attr aircode ! -attr material_id", "General"}, // TODO: same category of test, different test
        {"All regions have material & LOS", "search /all -type region ! -attr aircode ! -attr los", "General"}, // TODO: same category of test, different test
        // TODO: missing "No errors when top level drawn"
        {"No null region", "gqa -Ao -g4mm,4mm -t0.3mm all", "General"},
        // TODO: missing "BoTs are valid"
        // TODO: missing "Air does not stick out"
        {"Overlaps cleaned to 4mm gridsize with 0.3mm tolerance", "gqa -Ao -g32mm,4mm -t0.3mm all", "General"},
        {"Duplicate ID check", "lc -d all", "General"},
        {"No matrices", "search /all ! -matrix IDN", "General"},
        // TODO: missing "Title"
        // TODO: missing "Ground plane at z=0"
        {"No regions have aircodes (except actual air regions)", "search /all -type region -attr aircode", "General"}
    };
};

#endif // VVWIDGET_H