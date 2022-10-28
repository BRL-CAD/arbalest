#include "VerificationValidationWidget.h"
#include <Document.h>
#include "MainWindow.h"
using Result = VerificationValidation::Result;
using DefaultTests = VerificationValidation::DefaultTests;
using Parser = VerificationValidation::Parser;

#define SHOW_ERROR_POPUP true

// TODO: if checksum doesn't match current test file, notify user

VerificationValidationWidget::VerificationValidationWidget(MainWindow* mainWindow, Document* document, QWidget* parent) : document(document), testList(new QListWidget()), resultTable(new QTableWidget()), selectTestsDialog(new QDialog()), statusBar(nullptr), mainWindow(mainWindow) {
    QString dbName = "untitled" + QString::number(document->getDocumentId()) + ".atr";
    try { dbConnect(dbName); } catch (const std::runtime_error& e) { throw e; }
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
    // get the number of tests from db
    QString totalTests = 0;
    QSqlQuery* q = dbExec("SELECT COUNT(*) FROM Tests");
    if (q && q->next())
        totalTests = q->value(0).toString();

    // run through every test
    size_t testsRun = 0;
    QString status = "Finished running and parsing " + QString::number(++testsRun) + "/" + totalTests + " tests";
    q = dbExec("SELECT id, testCommand FROM Tests");
    while(q && q->next()) {
        statusBar->showMessage(status);
        QString testID = q->value(0).toString();
        QString testCommand = q->value(1).toString();
        const QString* terminalOutput = runTest(testCommand);

        QString executableName = testCommand.split(' ').first();
        Result* result = nullptr;
        // find proper parser
        if (QString::compare(executableName, "search", Qt::CaseInsensitive) == 0)
            result = Parser::search(testCommand, terminalOutput);

        // if parser hasn't been implemented, default
        if (!result) {
            result = new Result;
            result->resultCode = Result::Code::UNPARSEABLE;
        }

        QString resultCode = QString::number(result->resultCode);
        
        // insert results into db
        QSqlQuery* q2 = new QSqlQuery(getDatabase());
        q2->prepare("INSERT INTO TestResults (modelID, testID, resultCode, terminalOutput) VALUES (?,?,?,?)");
        q2->addBindValue(modelID);
        q2->addBindValue(testID);
        q2->addBindValue(resultCode);
        q2->addBindValue((terminalOutput) ? *terminalOutput : "");
        dbExec(q2);

        QString testResultID = q2->lastInsertId().toString();

        // insert issues into db
        for (Result::ObjectIssue currentIssue : result->issues) {
            q2 = new QSqlQuery(getDatabase());
            q2->prepare("INSERT INTO ObjectIssue (objectName, issueDescription) VALUES (?,?)");
            q2->addBindValue(currentIssue.objectName);
            q2->addBindValue(currentIssue.issueDescription);
            dbExec(q2);

            QString objectIssueID = q2->lastInsertId().toString();
            q2 = new QSqlQuery(getDatabase());
            q2->prepare("INSERT INTO Issues (testResultID, objectIssueID) VALUES (?,?)");
            q2->addBindValue(testResultID);
            q2->addBindValue(objectIssueID);
            dbExec(q2);
        }

        showResult(testResultID);

        status = "Finished running " + QString::number(++testsRun) + "/" + totalTests + " tests";
    }
}

void VerificationValidationWidget::loadATRFile(const QString& filepath) {
    dbConnect(filepath);

}

void VerificationValidationWidget::dbConnect(const QString dbName) {
    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
        throw std::runtime_error("[Verification & Validation] ERROR: sqlite is not available");

    this->dbName = dbName;
    QString* fp = document->getFilePath();
    if (fp) this->dbName = fp->split("/").last() + ".atr";
    dbConnectionName = this->dbName + "-connection";

    // check if SQL connection already open
    QSqlDatabase db = QSqlDatabase::database(dbConnectionName, false);
    // TODO: instead of throwing + popping up error, open correct document
    if (db.isOpen()) {
        const std::unordered_map<int, Document*>* documents = mainWindow->getDocuments();
        Document* correctDocument = nullptr;
        Document* doc;
        for (auto it = documents->begin(); it != documents->end(); it++) {
            doc = it->second;
            if (doc && doc != this->document && doc->getVerificationValidationWidget()->getDBConnectionName() == dbConnectionName) {
                correctDocument = doc;
                break;
            }
        }
        if (correctDocument) mainWindow->getDocumentArea()->setCurrentIndex(correctDocument->getTabIndex());
         // TODO: close this Document
         // TODO: think about what happens whenever you open untitled files, close arbalest, reopen untitled files
        throw std::runtime_error("[Verification & Validation] ERROR: SQL connection already exists");
    }
    db = QSqlDatabase::addDatabase("QSQLITE", dbConnectionName);
    db.setDatabaseName(this->dbName);

    if (!db.open() || !db.isOpen())
        throw std::runtime_error("[Verification & Validation] ERROR: db failed to open: " + db.lastError().text().toStdString());
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

void VerificationValidationWidget::dbPopulateDefaults() {
    QSqlQuery* q;
    QString md5Checksum = "TODO: HASH USING BRLCAD INTERFACE";

    // if Model table empty, assume new db and insert model info
    q = new QSqlQuery(getDatabase());
    q->prepare("SELECT id FROM Model WHERE filePath=?");
    q->addBindValue(this->dbName);
    dbExec(q, !SHOW_ERROR_POPUP);
    if (!q->next()) {
        q = new QSqlQuery(getDatabase());
        q->prepare("INSERT INTO Model (filepath, md5Checksum) VALUES (?, ?)");
        q->addBindValue(this->dbName);
        q->addBindValue(md5Checksum);
        dbExec(q);   
        modelID = q->lastInsertId().toString();
    } else {
        modelID = q->value(0).toString();
    }

    // if Tests table empty, new db and insert tests
    // note: this doesn't repopulate deleted tests, unless all tests deleted
    q = dbExec("SELECT id FROM Tests", !SHOW_ERROR_POPUP);
    if (!q->next()) {
        for (int i = 0; i < DefaultTests::allTests.size(); i++) {
            q->prepare("INSERT INTO Tests (testName, testCommand) VALUES (?, ?)");
            q->addBindValue(DefaultTests::allTests[i].testName);
            q->addBindValue(DefaultTests::allTests[i].testCommand);
            dbExec(q);

            QString testID = q->lastInsertId().toString();

            q->prepare("INSERT INTO TestsSuites (suiteName) VALUES (?)");
            q->addBindValue(DefaultTests::allTests[i].suiteName);
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
    columnLabels << "   " << "Test Name" << "Description" << "Object Path";
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
        popup("[Verification & Validation]\nERROR: query failed to execute: " + query->lastError().text() + "\n\n" + command);
    return query;
}

void VerificationValidationWidget::dbExec(QSqlQuery*& query, bool showErrorPopup) {
    query->exec();
    if (showErrorPopup && !query->isActive())
        popup("[Verification & Validation]\nERROR: query failed to execute: " + query->lastError().text() + "\n\n" + query->lastQuery());
}

void VerificationValidationWidget::resizeEvent(QResizeEvent* event) {
    resultTable->setColumnWidth(0, this->width() * 0.025);
    resultTable->setColumnWidth(1, this->width() * 0.125);
    resultTable->setColumnWidth(2, this->width() * 0.60);
    resultTable->setColumnWidth(3, this->width() * 0.25);

    QHBoxWidget::resizeEvent(event);
}

void VerificationValidationWidget::showResult(const QString& testResultID) {
    QSqlQuery* q = new QSqlQuery(getDatabase());
    q->prepare("SELECT Tests.testName, TestResults.resultCode, TestResults.terminalOutput FROM Tests INNER JOIN TestResults ON Tests.id=TestResults.testID WHERE TestResults.id = ?");
    q->addBindValue(testResultID);
    dbExec(q);

    if (!q->next()) {
        popup("Failed to show Test Result #" + testResultID);
        return;
    }

    QString testName = q->value(0).toString();
    int resultCode = q->value(1).toInt();
    QString terminalOutput = q->value(2).toString();

    QSqlQuery* q2 = new QSqlQuery(getDatabase());
    q2->prepare("SELECT objectIssueID FROM Issues WHERE testResultID = ?");
    q2->addBindValue(testResultID);
    dbExec(q2, !SHOW_ERROR_POPUP);

    while (q2->next()) {
        QString objectIssueID = q2->value(0).toString();

        QSqlQuery* q3 = new QSqlQuery(getDatabase());
        q3->prepare("SELECT objectName, issueDescription FROM ObjectIssue WHERE id = ?");
        q3->addBindValue(objectIssueID);
        dbExec(q3);

        if (!q3->next()) {
            popup("Failed to retrieve Object Issue #" + objectIssueID);
            return;
        }

        QString objectName = q3->value(0).toString();
        QString issueDescription = q3->value(1).toString();

        resultTable->insertRow(resultTable->rowCount());

        QString iconPath = "";
        if (resultCode == VerificationValidation::Result::Code::UNPARSEABLE)
            iconPath = ":/icons/unparseable.png";
        else if (resultCode == VerificationValidation::Result::Code::FAILED)
            iconPath = ":/icons/error.png";
        else if (resultCode == VerificationValidation::Result::Code::WARNING)
            iconPath = ":/icons/warning.png";
        else if (resultCode == VerificationValidation::Result::Code::PASSED)
            iconPath = ":/icons/passed.png";

        resultTable->setItem(resultTable->rowCount()-1, RESULT_CODE_COLUMN, new QTableWidgetItem(QIcon(iconPath), iconPath));
        resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem(testName));
        resultTable->setItem(resultTable->rowCount()-1, DESCRIPTION_COLUMN, new QTableWidgetItem(issueDescription));
        resultTable->setItem(resultTable->rowCount()-1, OBJPATH_COLUMN, new QTableWidgetItem(objectName));
    }
}