#include "TestsWidget.h"

TestsWidget::TestsWidget(Document* document, QWidget* parent) : document(document), QTableWidget(parent) {
    /* OPEN UP AN IN-MEMORY SQLite DATABASE */
    const QString DRIVER("QSQLITE");
    if (QSqlDatabase::isDriverAvailable(DRIVER))
        printf("sqlite is available\n");
    else
        printf("sqlite is NOT available\n");

    QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);

    db.setDatabaseName("tmp.sqlite");

    if (!db.open()) {
        std::cout << "ERROR: " << db.lastError().text().toStdString() << std::endl;
    }

    /* CREATE OUR SQLite TABLE */
    QSqlQuery* query = new QSqlQuery("CREATE TABLE issues (id INTEGER PRIMARY KEY, object_name TEXT NOT NULL, severity TEXT CHECK( severity in ('E', 'W', 'I') ) NOT NULL, description TEXT DEFAULT NULL)", db);
    if (!query->isActive())
        std::cout << "ERROR: on q1 creation: " << query->lastError().text().toStdString() << std::endl;

    
    QSqlQuery* query2 = new QSqlQuery("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/foo.r', 'W', 'object name implies region, but is not a region')", db);
    if (!query2->isActive())
        std::cout << "ERROR: on q2 creation: " << query2->lastError().text().toStdString() << std::endl;

    QSqlQuery* query3 = new QSqlQuery("INSERT INTO issues(object_name, severity, description) VALUES('/all.g/bar.r', 'E', 'null combination')", db);
    if (!query3->isActive())
        std::cout << "ERROR: on q3 creation: " << query3->lastError().text().toStdString() << std::endl;

    /* LOOK UP ERROR ISSUES */
    QSqlQuery* query4 = new QSqlQuery("SELECT object_name, severity, description FROM issues WHERE severity = 'E'", db);
    if (!query4->isActive())
        std::cout << "ERROR: on q4 creation: " << query4->lastError().text().toStdString() << std::endl;

    std::cout << "--------------" << std::endl;

    /* PRINT ONE OF THEM TO CONSOLE */
    if (query4->first()) {
        switch (query4->value(1).toString().toStdString().c_str()[0]) {
        case 'E':
            std::cout << "ERROR: ";
            break;
        case 'W':
            std::cout << "WARNING: ";
            break;
        case 'I':
        default:
            std::cout << "INFO: ";
            break;
        }
        std::cout << query4->value(0).toString().toStdString();
        if (!query4->value(2).toString().isEmpty())
            std::cout << " " << query4->value(2).toString().toStdString() << std::endl;
        else
            std::cout << std::endl;

    }
    else {
        std::cout << "no errors found" << std::endl;
    }

    //////
    setRowCount(10);
    setColumnCount(3);

    /* LOOK UP ALL ISSUES */
    QSqlQuery* query5 = new QSqlQuery("SELECT object_name, severity, description FROM issues", db);
    if (!query5->isActive())
        std::cout << "ERROR: on q5 creation: " << query5->lastError().text().toStdString() << std::endl;

    /* INSERT ISSUES INTO A TABLE */
    size_t row = 0;
    while (query5->next()) {
        char typechar = query5->value(1).toString().toStdString().c_str()[0];
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
        QTableWidgetItem* item1 = new QTableWidgetItem(tr("%1").arg(query5->value(0).toString().toStdString().c_str()));
        setItem(row, 1, item1);
        QTableWidgetItem* item2 = new QTableWidgetItem(tr("%1").arg(query5->value(2).toString().toStdString().c_str()));
        setItem(row, 2, item2);

        row++;
    }
}