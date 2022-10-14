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

    // init functions
    void dbConnect();
    void dbInitTables();
    void dbPopulateTables();
    void dbInitDummyData();
    void setupUI();

    // database functions
    QSqlQuery* dbExec(QString command, bool showErrorPopup = true);
    QSqlDatabase getDatabase() const {
        return QSqlDatabase::database(dbConnectionName);
    }

    // UI functions
    void popup(QString message);

    // events
    void resizeEvent(QResizeEvent* event) override;

    // constant defaults
    // TODO: default tests currently don't have suiteName... do we want this?
    const std::vector<std::vector<QString>> defaultTests = {
        {"No mis-matched dupliate IDs", "lc -m all"},
        {"No nested regions", "search /all -type region -below -type region"},
        {"No empty combos", "search /all -nnodes 0"},
        {"No solids outside of regions", "search /all ! -below -type region -type shape"},
        {"All BoTs are volume mode (should return nothing)", "search all -type bot ! -type volume"},
        {"No BoTs are left hand orientation", "search all -type bot -param orient=lh"}, // TODO: this command can run faster if use unix
        {"All regions have material & LOS", "search /all -type region ! -attr aircode ! -attr material_id"}, // TODO: same category of test, different test
        {"All regions have material & LOS", "search /all -type region ! -attr aircode ! -attr los"}, // TODO: same category of test, different test
        // TODO: missing "No errors when top level drawn"
        {"No null region", "gqa -Ao -g4mm,4mm -t0.3mm all"},
        // TODO: missing "BoTs are valid"
        // TODO: missing "Air does not stick out"
        {"Overlaps cleaned to 4mm gridsize with 0.3mm tolerance", "gqa -Ao -g32mm,4mm -t0.3mm all"},
        {"Duplicate ID check", "lc -d all"},
        {"No matrices", "search /all ! -matrix IDN"},
        // TODO: missing "Title"
        // TODO: missing "Ground plane at z=0"
        {"No regions have aircodes (except actual air regions)", "search /all -type region -attr aircode"}
    };
};


#endif // VVWIDGET_H