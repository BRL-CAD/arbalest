#include "TestsWidget.h"
#include <Document.h>

void TestsWidget::dbInit() {
    const QString DRIVER("QSQLITE");
    if (!QSqlDatabase::isDriverAvailable(DRIVER))
        printf("TODO: popup error sqlite is NOT available\n");

    db = QSqlDatabase::addDatabase(DRIVER);

    QString dbName = document->getFilePath()->split("/").last() + ".sqlite";
    db.setDatabaseName(dbName);

    if (!db.open())
        std::cout << "TODO: popup ERROR: " << db.lastError().text().toStdString() << std::endl;
}

QSqlQuery* TestsWidget::dbExec(QString command) {
    QSqlQuery* query = new QSqlQuery(command, db);
    if (!query->isActive())
        std::cout << "TODO: popup ERROR:" << query->lastError().text().toStdString() << std::endl;
    return query;
}

TestsWidget::TestsWidget(Document* document, QWidget* parent) : document(document), QTableWidget(parent) {
    dbInit();

    QSqlQuery* qResult = dbExec("CREATE TABLE issues (id INTEGER PRIMARY KEY, object_name TEXT NOT NULL, severity TEXT CHECK( severity in ('E', 'W', 'I') ) NOT NULL, description TEXT DEFAULT NULL)");
    qResult = dbExec("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/foo.r', 'W', 'object name implies region, but is not a region')");
    qResult = dbExec("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/bar.r', 'E', 'null combination')");
    qResult = dbExec("SELECT object_name, severity, description FROM issues WHERE severity = 'E'");    

    //////
    setRowCount(10);
    setColumnCount(3);

    /* LOOK UP ALL ISSUES */
    qResult = dbExec("SELECT object_name, severity, description FROM issues");

    /* INSERT ISSUES INTO A TABLE */
    size_t row = 0;
    while (qResult->next()) {
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
        setItem(row, 0, item0);
        QTableWidgetItem* item1 = new QTableWidgetItem(tr("%1").arg(qResult->value(0).toString().toStdString().c_str()));
        setItem(row, 1, item1);
        QTableWidgetItem* item2 = new QTableWidgetItem(tr("%1").arg(qResult->value(2).toString().toStdString().c_str()));
        setItem(row, 2, item2);

        row++;
    }
}