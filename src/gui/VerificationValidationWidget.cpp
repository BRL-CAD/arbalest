#include "VerificationValidationWidget.h"
#include <Document.h>

VerificationValidationWidget::VerificationValidationWidget(Document* document, QWidget* parent) : document(document), list(new QListWidget()), table(new QTableWidget()) {
    dbInit();
    QSqlQuery* qResult;
    //////
    table->setRowCount(10);
    table->setColumnCount(3);
    QStringList* columnLabels = new QStringList();
    *columnLabels << "Severity" << "Object Name" << "Description";
    table->setHorizontalHeaderLabels(*columnLabels);

    if (!getDatabase().tables().contains("issues")) {
        qResult = dbExec("CREATE TABLE issues (id INTEGER PRIMARY KEY, object_name TEXT NOT NULL, severity TEXT CHECK( severity in ('E', 'W', 'I') ) NOT NULL, description TEXT DEFAULT NULL)");
        qResult = dbExec("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/foo.r', 'W', 'object name implies region, but is not a region')");
        qResult = dbExec("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/bar.r', 'E', 'null combination')");
        qResult = dbExec("SELECT object_name, severity, description FROM issues WHERE severity = 'E'");
    }

    qResult = dbExec("SELECT object_name, severity, description FROM issues");
    size_t row = 0;
    while (qResult && qResult->next()) {
        char typechar = qResult->value(1).toString().toStdString().c_str()[0];
        const char* type = NULL;
        switch (typechar) {
        case 'E':
            type = "ERROR";
            break;
        case 'W':
            type = "WARNING";
            break;
        case 'I':
        default:
            type = "INFO";
            break;
        }
        QTableWidgetItem* item0 = new QTableWidgetItem(tr("%1").arg(type));
        table->setItem(row, 0, item0);
        QTableWidgetItem* item1 = new QTableWidgetItem(tr("%1").arg(qResult->value(0).toString().toStdString().c_str()));
        table->setItem(row, 1, item1);
        QTableWidgetItem* item2 = new QTableWidgetItem(tr("%1").arg(qResult->value(2).toString().toStdString().c_str()));
        table->setItem(row, 2, item2);

        row++;
    }

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

VerificationValidationWidget::~VerificationValidationWidget() {
    QSqlDatabase::removeDatabase(dbConnectionName);
}

void VerificationValidationWidget::dbInit() {
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