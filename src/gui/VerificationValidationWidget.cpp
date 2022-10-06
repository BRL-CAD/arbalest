#include "VerificationValidationWidget.h"
#include <Document.h>

VerificationValidationWidget::VerificationValidationWidget(Document* document, QWidget* parent) : document(document), list(new QListWidget()), table(new QTableWidget()) {
    dbConnect();
    dbInitTables();
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
    QSqlQuery* qResult;
    if (!getDatabase().tables().contains("Model"))
        qResult = dbExec("CREATE TABLE Model (id INTEGER PRIMARY KEY, filepath TEXT NOT NULL UNIQUE, md5Checksum TEXT NOT NULL, modelTestResultID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("ModelTestResults"))
        qResult = dbExec("CREATE TABLE ModelTestResults (id INTEGER PRIMARY KEY, testResultID INTEGER NOT NULL UNIQUE)");
    if (!getDatabase().tables().contains("Tests"))
        qResult = dbExec("CREATE TABLE Tests (id INTEGER PRIMARY KEY, testName TEXT NOT NULL, testCommand TEXT NOT NULL UNIQUE)");
    if (!getDatabase().tables().contains("TestResults"))
        qResult = dbExec("CREATE TABLE TestResults (id INTEGER PRIMARY KEY, testID INTEGER NOT NULL, resultCode TEXT, terminalOutput TEXT)");
    if (!getDatabase().tables().contains("Issues"))
        qResult = dbExec("CREATE TABLE Issues (id INTEGER PRIMARY KEY, testResultID INTEGER NOT NULL, objectIssueID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("ObjectIssue"))
        qResult = dbExec("CREATE TABLE ObjectIssue (id INTEGER PRIMARY KEY, objectName TEXT NOT NULL, issueDescription TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestsSuites"))
        qResult = dbExec("CREATE TABLE TestsSuites (id INTEGER PRIMARY KEY, suiteName TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestsInSuite"))
        qResult = dbExec("CREATE TABLE TestsInSuite (id INTEGER PRIMARY KEY, testSuiteID INTEGER NOT NULL, testID INTEGER NOT NULL)");
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

QSqlQuery* VerificationValidationWidget::dbExec(QString command) {
    QSqlQuery* query = new QSqlQuery(command, getDatabase());
    if (!query->isActive())
        popupError("[Verification & Validation] ERROR: query failed to execute: " + query->lastError().text());
    return query;
}

void VerificationValidationWidget::popupError(QString message) {
    QMessageBox* msgBox = new QMessageBox();
    msgBox->setText(message);
    msgBox->exec();
}