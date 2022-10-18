#include "VerificationValidationWidget.h"
#include <Document.h>

#define SHOW_ERROR_POPUP true
// TODO: if checksum doesn't match current test file, notify user

VerificationValidationWidget::VerificationValidationWidget(Document* document, QWidget* parent) : document(document), testList(new QListWidget()), resultTable(new QTableWidget()), selectTestsDialog(new QDialog()), statusBar(nullptr) {
    dbConnect();
    dbInitTables();
    dbPopulateDefaults();
    setupUI();
}

VerificationValidationWidget::~VerificationValidationWidget() {
    QSqlDatabase::removeDatabase(dbConnectionName);
}


void VerificationValidationWidget::showSelectTests() {
    statusBar->showMessage("Select tests to run...");
    selectTestsDialog->exec();
}

QString* VerificationValidationWidget::runTest(const QString& cmd) {
    struct ged *dbp;
    const QStringList tmp = cmd.split(QRegExp("\\s"), Qt::SkipEmptyParts);

    const char* cmdList[tmp.size() + 1];
    for (int i = 0; i < tmp.size(); i++) {
        char* cmdBuf = new char[tmp[i].size() + 1];
        strncpy(cmdBuf, tmp[i].toStdString().data(), tmp[i].size());
        cmdBuf[tmp[i].size()] = '\0';
        cmdList[i] = cmdBuf;
    }
    cmdList[tmp.size()] = NULL;
    
    QString filepath = *(document->getFilePath());
    if (!bu_file_exists((filepath).toStdString().c_str(), NULL)) {
        QString errorMsg = "[Verification & Validation] ERROR: [" + filepath + "] does not exist\n";
        popup(errorMsg);
        return nullptr;
    }

    dbp = ged_open("db", filepath.toStdString().c_str(), 1);
    ged_exec(dbp, tmp.size(), cmdList);
    QString* result = new QString(bu_vls_addr(dbp->ged_result_str));
    ged_close(dbp);

    return result;
}

void VerificationValidationWidget::runTests() {
    QString totalTests = 0;
    QSqlQuery* q = dbExec("SELECT COUNT(*) FROM Tests");
    if (q && q->next())
        totalTests = q->value(0).toString();

    size_t testsRun = 0;
    QString status = "Finished running " + QString::number(++testsRun) + "/" + totalTests + " tests";
    q = dbExec("SELECT id, testCommand FROM Tests");
    while(q && q->next()) {
        statusBar->showMessage(status);
        QString testID = q->value(0).toString();
        QString testCommand = q->value(1).toString();
        const QString* terminalOutput = runTest(testCommand);
        VerificationValidationResult* result = VerificationValidationParser::search(terminalOutput);
        QString resultCode = QString::number(result->resultCode);
        
        QSqlQuery* q2 = new QSqlQuery(getDatabase());
        q2->prepare("INSERT INTO TestResults (modelID, testID, resultCode, terminalOutput) VALUES (?,?,?,?)");
        q2->addBindValue(modelID);
        q2->addBindValue(testID);
        q2->addBindValue(resultCode);
        q2->addBindValue((terminalOutput) ? *terminalOutput : "");
        dbExec(q2);

        // TODO: add bad objects

        status = "Finished running " + QString::number(++testsRun) + "/" + totalTests + " tests";
    }
    // TODO: update GUI to show results of test
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

void VerificationValidationWidget::dbInitTables() {
    if (!getDatabase().tables().contains("Model"))
        dbExec("CREATE TABLE Model (id INTEGER PRIMARY KEY, filepath TEXT NOT NULL UNIQUE, sha256Checksum TEXT NOT NULL)");
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

void VerificationValidationWidget::dbPopulateDefaults() {
    QSqlQuery* q;
    QString sha256Checksum = "TODO: HASH SHA256 FROM OPENSSL";

    // if Model table empty, assume new db and insert model info
    q = new QSqlQuery(getDatabase());
    q->prepare("SELECT id FROM Model WHERE filePath=?");
    q->addBindValue(dbName);
    dbExec(q, !SHOW_ERROR_POPUP);
    if (!q->next()) {
        q = new QSqlQuery(getDatabase());
        q->prepare("INSERT INTO Model (filepath, sha256Checksum) VALUES (?, ?)");
        q->addBindValue(dbName);
        q->addBindValue(sha256Checksum);
        dbExec(q);   
        modelID = q->lastInsertId().toString();
    } else {
        modelID = q->value(0).toString();
    }

    // if Tests table empty, new db and insert tests
    // note: this doesn't repopulate deleted tests, unless all tests deleted
    q = dbExec("SELECT id FROM Tests", !SHOW_ERROR_POPUP);
    if (!q->next()) {
        for (int i = 0; i < defaultTests.size(); i++) {
            q->prepare("INSERT INTO Tests (testName, testCommand) VALUES (?, ?)");
            q->addBindValue(defaultTests[i].testName);
            q->addBindValue(defaultTests[i].testCommand);
            dbExec(q);

            QString testID = q->lastInsertId().toString();

            q->prepare("INSERT INTO TestsSuites (suiteName) VALUES (?)");
            q->addBindValue(defaultTests[i].suiteName);
            dbExec(q);

            QString testSuiteID = q->lastInsertId().toString();

            q->prepare("INSERT INTO TestsInSuite (testID, testSuiteID) VALUES (?,?)");
            q->addBindValue(testID);
            q->addBindValue(testSuiteID);
            dbExec(q);
        }
    }
}

void VerificationValidationWidget::setupUI() {
    // setup result table's column headers
    QStringList columnLabels;
    columnLabels << "   " << "   " << "Test Name" << "Description" << "Object Path";
    resultTable->setColumnCount(columnLabels.size());
    resultTable->setHorizontalHeaderLabels(columnLabels);
    resultTable->verticalHeader()->setVisible(false);
    resultTable->horizontalHeader()->setStretchLastSection(true);
    resultTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    addWidget(resultTable);

    // populate checkbox list with tests
    QStringList tests;
    tests << "test 1" << "test 2" << "test 3" << "test 4";
    testList->addItems(tests);

    QListWidgetItem* item = 0;
    for (int i = 0; i < testList->count(); i++) {
        item = testList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    // format and populate Select Tests dialog box
    selectTestsDialog->setModal(true);
    selectTestsDialog->setWindowTitle("Select Tests");
    selectTestsDialog->setLayout(new QVBoxLayout);
    selectTestsDialog->layout()->addWidget(testList);

    QDialogButtonBox* buttonOptions = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    selectTestsDialog->layout()->addWidget(buttonOptions);
    connect(buttonOptions, &QDialogButtonBox::accepted, selectTestsDialog, &QDialog::accept);
    connect(buttonOptions, &QDialogButtonBox::rejected, selectTestsDialog, &QDialog::reject);
}

QSqlQuery* VerificationValidationWidget::dbExec(QString command, bool showErrorPopup) {
    QSqlQuery* query = new QSqlQuery(command, getDatabase());
    if (showErrorPopup && !query->isActive())
        popup("[Verification & Validation] ERROR: query failed to execute: " + query->lastError().text());
    return query;
}

void VerificationValidationWidget::dbExec(QSqlQuery*& query, bool showErrorPopup) {
    query->exec();
    if (showErrorPopup && !query->isActive())
        popup("[Verification & Validation] ERROR: query failed to execute: " + query->lastError().text());
}

void VerificationValidationWidget::resizeEvent(QResizeEvent* event) {
    resultTable->setColumnWidth(0, this->width() * 0.025);
    resultTable->setColumnWidth(1, this->width() * 0.025);
    resultTable->setColumnWidth(2, this->width() * 0.10);
    resultTable->setColumnWidth(3, this->width() * 0.60);
    resultTable->setColumnWidth(4, this->width() * 0.25);

    QHBoxWidget::resizeEvent(event);
}