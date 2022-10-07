#include "VerificationValidationWidget.h"
#include <Document.h>

VerificationValidationWidget::VerificationValidationWidget(Document* document, QWidget* parent) : document(document), list(new QListWidget()), table(new QTableWidget()) {
    dbConnect();
    dbInitTables();
    dbPopulateTables();
    dbInitDummyData();

    table->setRowCount(10);
    table->setColumnCount(3);
    
    // QStringList* columnLabels = new QStringList();
    // *columnLabels << "Severity" << "Object Name" << "Description";
    // table->setHorizontalHeaderLabels(*columnLabels);

    // QSqlQuery* qResult;
    // qResult = dbExec("SELECT object_name, severity, description FROM issues");
    // size_t row = 0;
    // while (qResult && qResult->next()) {
    //     char typechar = qResult->value(1).toString().toStdString().c_str()[0];
    //     const char* type = NULL;
    //     switch (typechar) {
    //     case 'E':
    //         type = "ERROR";
    //         break;
    //     case 'W':
    //         type = "WARNING";
    //         break;
    //     case 'I':
    //     default:
    //         type = "INFO";
    //         break;
    //     }
    //     QTableWidgetItem* item0 = new QTableWidgetItem(tr("%1").arg(type));
    //     table->setItem(row, 0, item0);
    //     QTableWidgetItem* item1 = new QTableWidgetItem(tr("%1").arg(qResult->value(0).toString().toStdString().c_str()));
    //     table->setItem(row, 1, item1);
    //     QTableWidgetItem* item2 = new QTableWidgetItem(tr("%1").arg(qResult->value(2).toString().toStdString().c_str()));
    //     table->setItem(row, 2, item2);

    //     row++;
    // }

    QStringList* tests = new QStringList();
    *tests << "test 1" << "test 2" << "test 3" << "test 4";
    list->addItems(*tests);

    QListWidgetItem* item = 0;
    for (int i = 0; i < list->count(); i++) {
        item = list->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    addWidget(list);
    addWidget(table);

    getBoxLayout()->setStretchFactor(list, 1);
    getBoxLayout()->setStretchFactor(table, 3);
}

void VerificationValidationWidget::dbInitTables() {
    if (!getDatabase().tables().contains("Model"))
        dbExec("CREATE TABLE Model (id INTEGER PRIMARY KEY, filepath TEXT NOT NULL UNIQUE, md5Checksum TEXT NOT NULL)");
    if (!getDatabase().tables().contains("Tests"))
        dbExec("CREATE TABLE Tests (id INTEGER PRIMARY KEY, testName TEXT NOT NULL, testCommand TEXT NOT NULL UNIQUE)");
    if (!getDatabase().tables().contains("TestResults"))
        dbExec("CREATE TABLE TestResults (id INTEGER PRIMARY KEY, modelID INTEGER NOT NULL, testID INTEGER NOT NULL, resultCode TEXT, terminalOutput TEXT)");
    if (!getDatabase().tables().contains("Issues"))
        dbExec("CREATE TABLE Issues (id INTEGER PRIMARY KEY, testResultID INTEGER NOT NULL, objectIssueID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("ObjectIssue"))
        dbExec("CREATE TABLE ObjectIssue (id INTEGER PRIMARY KEY, objectName TEXT NOT NULL, issueDescription TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestsSuites"))
        dbExec("CREATE TABLE TestsSuites (id INTEGER PRIMARY KEY, suiteName TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestsInSuite"))
        dbExec("CREATE TABLE TestsInSuite (id INTEGER PRIMARY KEY, testSuiteID INTEGER NOT NULL, testID INTEGER NOT NULL)");
}

void VerificationValidationWidget::dbPopulateTables() {
    const bool SHOW_ERROR_POPUP = true;

    QSqlQuery* qResult;

    // if Model table empty, assume new db and insert model info
    QString cmd = "SELECT id FROM Model WHERE filePath='" + dbName + "'";
    qResult = dbExec(cmd, !SHOW_ERROR_POPUP);
    if (!qResult->next()) {
        cmd = "INSERT INTO Model (filepath, md5Checksum) VALUES ('" + dbName + "', '" + "TODO: DECIDE IF NEED HASH" + "')";
        dbExec(cmd);
    }

    // if Tests table empty, new db and insert tests
    // note: this doesn't repopulate deleted tests, unless all tests deleted
    qResult = dbExec("SELECT id FROM Tests", !SHOW_ERROR_POPUP);
    if (!qResult->next()) {
        const vector<pair<const QString, const QString>> defaultTests = {
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

        for (int i = 0; i < defaultTests.size(); i++) {
            cmd = "INSERT INTO Tests (testName, testCommand) VALUES ('" + defaultTests[i].first + "', '" + defaultTests[i].second + "')";
            dbExec(cmd);
        }
    }

    // create TestResult entry for every Test that doesn't have a corresponding TestResult
    cmd = "SELECT id FROM Model WHERE filePath='" + dbName + "'";
    qResult = dbExec(cmd);
    qResult->next();
    QString modelID = qResult->value(0).toString();

    qResult = dbExec("SELECT id FROM Tests WHERE NOT EXISTS (SELECT 1 FROM TestResults WHERE TestResults.testId=Tests.id)", !SHOW_ERROR_POPUP);
    while (qResult && qResult->next()) {
        cmd = "INSERT INTO TestResults (modelID, testID) VALUES (" + modelID + ", " + qResult->value(0).toString() + ")";
        dbExec(cmd);
    }
}

void VerificationValidationWidget::dbInitDummyData() {

}

VerificationValidationWidget::~VerificationValidationWidget() {
    QSqlDatabase::removeDatabase(dbConnectionName);
}

void VerificationValidationWidget::dbConnect() {
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        throw std::runtime_error("[Verification & Validation] ERROR: sqlite is not available");
        return;
    }

    dbName = "tmpfile.sqlite"; 
    if (document->getFilePath()) dbName = document->getFilePath()->split("/").last() + ".sqlite";
    dbConnectionName = dbName + "-connection";

    // check if SQL connection already open
    QSqlDatabase db = QSqlDatabase::database(dbConnectionName, false);
    // TODO: instead of throwing + popping up error, open correct document
    if (db.isOpen())
        throw std::runtime_error("[Verification & Validation] ERROR: SQL connection already exists");
    
    //// TODO: opening multiple new files crashes
    //// TODO: whenever user saves, sqlite file name should be updated from tmpfile.sqlite to <newfilename>.sqlite
    db = QSqlDatabase::addDatabase("QSQLITE", dbConnectionName);
    db.setDatabaseName(dbName);

    if (!db.open() || !db.isOpen())
        throw std::runtime_error("[Verification & Validation] ERROR: db failed to open: " + db.lastError().text().toStdString());
}

QSqlQuery* VerificationValidationWidget::dbExec(QString command, bool showErrorPopup) {
    QSqlQuery* query = new QSqlQuery(command, getDatabase());
    if (showErrorPopup && !query->isActive())
        popupError("[Verification & Validation] ERROR: query failed to execute: " + query->lastError().text());
    return query;
}

void VerificationValidationWidget::popupError(QString message) {
    QMessageBox* msgBox = new QMessageBox();
    msgBox->setText(message);
    msgBox->exec();
}