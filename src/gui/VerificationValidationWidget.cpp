#include "VerificationValidationWidget.h"
#include <Document.h>

#define SHOW_ERROR_POPUP true
// TODO: if checksum doesn't match current test file, notify user

VerificationValidationWidget::VerificationValidationWidget(Document* document, QWidget* parent) : document(document), testList(new QListWidget()), resultTable(new QTableWidget()) {
    dbConnect();
    dbInitTables();
    dbPopulateTables();
    dbInitDummyData();
    setupUI();
}

void VerificationValidationWidget::dbInitTables() {
    if (!getDatabase().tables().contains("Model"))
        dbExec("CREATE TABLE Model (id INTEGER PRIMARY KEY, filepath TEXT NOT NULL UNIQUE, md5Checksum TEXT NOT NULL)");
    if (!getDatabase().tables().contains("Tests"))
        dbExec("CREATE TABLE Tests (id INTEGER PRIMARY KEY, testName TEXT NOT NULL, testCommand TEXT NOT NULL UNIQUE)");
    if (!getDatabase().tables().contains("TestResults"))
        dbExec("CREATE TABLE TestResults (id INTEGER PRIMARY KEY, modelID INTEGER NOT NULL, testID INTEGER NOT NULL, resultCode TEXT, terminalOutput TEXT)");
    if (!getDatabase().tables().contains("Issues"))
        dbExec("CREATE TABLE Issues (id INTEGER PRIMARY KEY, testID INTEGER NOT NULL, objectIssueID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("ObjectIssue"))
        dbExec("CREATE TABLE ObjectIssue (id INTEGER PRIMARY KEY, objectName TEXT NOT NULL, issueDescription TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestsSuites"))
        dbExec("CREATE TABLE TestsSuites (id INTEGER PRIMARY KEY, suiteName TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestsInSuite"))
        dbExec("CREATE TABLE TestsInSuite (id INTEGER PRIMARY KEY, testSuiteID INTEGER NOT NULL, testID INTEGER NOT NULL)");
}

void VerificationValidationWidget::dbPopulateTables() {
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
        // TODO: default tests currently don't have suiteName... do we want this?
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
    modelID = qResult->value(0).toString();

    qResult = dbExec("SELECT id FROM Tests WHERE NOT EXISTS (SELECT * FROM TestResults WHERE TestResults.testID=Tests.id)", !SHOW_ERROR_POPUP);
    while (qResult && qResult->next()) {
        cmd = "INSERT INTO TestResults (modelID, testID) VALUES (" + modelID + ", " + qResult->value(0).toString() + ")";
        dbExec(cmd);
    }
}

void VerificationValidationWidget::dbInitDummyData() {

}

void VerificationValidationWidget::setupUI() {    
    QStringList columnLabels;
    columnLabels << "Result Code" << "Problem Object/Region" << "Problem Description";
    columnLabels << "Test ID" << "Test Suite" << "Test Name" << "Test Command";
    columnLabels << "Terminal Output"; // TODO: move to popup?
    resultTable->setColumnCount(columnLabels.size());
    resultTable->setHorizontalHeaderLabels(columnLabels);

    // populate UI w/ db entries
    QSqlQuery* qTestResults;
    QString cmd = "SELECT resultCode, testID, terminalOutput FROM TestResults WHERE modelID=" + modelID;
    qTestResults = dbExec(cmd);
    int numRows = 0;
    while (qTestResults && qTestResults->next()) {
        QString resultCode = qTestResults->value(0).toString();
        QString testID = qTestResults->value(1).toString();
        QString terminalOutput = qTestResults->value(2).toString();

        cmd = "SELECT testName, testCommand FROM Tests WHERE id=" + testID;
        QSqlQuery* qResult = dbExec(cmd);
        qResult->next();
        
        QString testName = qResult->value(0).toString();
        QString testCommand = qResult->value(1).toString();

        cmd = "SELECT suiteName FROM TestsSuites INNER JOIN TestsInSuite WHERE TestsSuites.id=TestsInSuite.testSuiteID AND TestsInSuite.testID=" + testID;
        qResult = dbExec(cmd);
        QString suiteName = (qResult->next()) ? qResult->value(0).toString() : "";

        cmd = "SELECT objectName, issueDescription FROM ObjectIssue INNER JOIN Issues WHERE Issues.testID=" + testID;
        qResult = dbExec(cmd);
        QString problemObjectName = "";
        QString issueDescription = "";
        if (qResult->next()) {
            problemObjectName = qResult->value(0).toString();
            issueDescription = qResult->value(1).toString();
        }

        resultTable->setRowCount(numRows + 1);
        // put everything onto view
        QTableWidgetItem* item0 = new QTableWidgetItem(tr("%1").arg(resultCode));
        resultTable->setItem(numRows, 0, item0);
        QTableWidgetItem* item1 = new QTableWidgetItem(tr("%1").arg(problemObjectName));
        resultTable->setItem(numRows, 1, item1);
        QTableWidgetItem* item2 = new QTableWidgetItem(tr("%1").arg(issueDescription));
        resultTable->setItem(numRows, 2, item2);
        QTableWidgetItem* item3 = new QTableWidgetItem(tr("%1").arg(testID));
        resultTable->setItem(numRows, 3, item3);
        QTableWidgetItem* item4 = new QTableWidgetItem(tr("%1").arg(suiteName));
        resultTable->setItem(numRows, 4, item4);
        QTableWidgetItem* item5 = new QTableWidgetItem(tr("%1").arg(testName));
        resultTable->setItem(numRows, 5, item5);
        QTableWidgetItem* item6 = new QTableWidgetItem(tr("%1").arg(testCommand));
        resultTable->setItem(numRows, 6, item6);
        QTableWidgetItem* item7 = new QTableWidgetItem(tr("%1").arg(terminalOutput));
        resultTable->setItem(numRows, 7, item7);
        
        ++numRows;
    }

    // TODO: make custom collapsible component (with check box)... consider reusing CollapsibleWidget... if not: https://stackoverflow.com/questions/11077793/is-there-a-standard-component-for-collapsible-panel-in-qt
    QStringList tests;
    tests << "test 1" << "test 2" << "test 3" << "test 4";
    testList->addItems(tests);

    QListWidgetItem* item = 0;
    for (int i = 0; i < testList->count(); i++) {
        item = testList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    addWidget(testList);
    addWidget(resultTable);

    getBoxLayout()->setStretchFactor(testList, 1);
    getBoxLayout()->setStretchFactor(resultTable, 3);
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
        popup("[Verification & Validation] ERROR: query failed to execute: " + query->lastError().text());
    return query;
}

void VerificationValidationWidget::popup(QString message) {
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}