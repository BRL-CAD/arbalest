#include "TestsWidget.h"
#include <Document.h>

TestsWidget::TestsWidget(Document* document, QWidget* parent) : document(document), list(new QListWidget()), table(new QTableWidget()) {
    dbInit();
    if (!db) return;

    QSqlQuery* qResult;

    if (!(qResult = dbExec("CREATE TABLE issues (id INTEGER PRIMARY KEY, object_name TEXT NOT NULL, severity TEXT CHECK( severity in ('E', 'W', 'I') ) NOT NULL, description TEXT DEFAULT NULL)"))) return;
    if (!(qResult = dbExec("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/foo.r', 'W', 'object name implies region, but is not a region')"))) return;
    if (!(qResult = dbExec("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/bar.r', 'E', 'null combination')"))) return;
    if (!(qResult = dbExec("SELECT object_name, severity, description FROM issues WHERE severity = 'E'"))) return;

    //////
    table->setRowCount(10);
    table->setColumnCount(3);
    QStringList* columnLabels = new QStringList();
    *columnLabels << "Severity" << "Object Name" << "Description";
    table->setHorizontalHeaderLabels(*columnLabels);

    /* LOOK UP ALL ISSUES */
    if (!(qResult = dbExec("SELECT object_name, severity, description FROM issues"))) return;

    /* INSERT ISSUES INTO A TABLE */
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

TestsWidget::~TestsWidget() {
    QSqlDatabase::removeDatabase(db->connectionName());
}

void TestsWidget::dbInit() {
    //const QString* DRIVER = new QString();
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        popupError("ERROR: sqlite is not available");
        return;
    }

    // TODO: include connection name so can have multiple open at a time
    // TODO: on destructor, close sql connection
    QString dbName = document->getFilePath()->split("/").last() + ".sqlite";
    db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", dbName + "-connection")); // connectionName = <dbName>-connection
    if (!QFile::exists(dbName)) {
        db->setDatabaseName(dbName);

        if (!db->open() || !db->isOpen()) {
            popupError("ERROR: " + db->lastError().text());
            return;
        }
    }
    else {
        // TODO: open existing
    }
}

QSqlQuery* TestsWidget::dbExec(QString command) {
    QSqlQuery* query = new QSqlQuery(command, *db);
    if (!query->isActive()) {
        popupError("ERROR: " + query->lastError().text());
        return nullptr;
    }

    return query;
}

void TestsWidget::popupError(QString message) {
    QMessageBox* msgBox = new QMessageBox();
    msgBox->setText(message);
    msgBox->exec();
}