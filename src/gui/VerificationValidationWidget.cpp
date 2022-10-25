#include "VerificationValidationWidget.h"
#include <Document.h>

#define SHOW_ERROR_POPUP true
// TODO: if checksum doesn't match current test file, notify user

VerificationValidationWidget::VerificationValidationWidget(Document* document, QWidget* parent) : document(document), testList(new QListWidget()), suiteList(new QListWidget()), test_sa(new QListWidget()), suite_sa(new  QListWidget()), resultTable(new QTableWidget()), selectTestsDialog(new QDialog()), statusBar(nullptr) {
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
        QString* terminalOutput = runTest(testCommand);

        if (!terminalOutput) {
            // TODO: run through parser to get resultCode
            QSqlQuery* q2 = new QSqlQuery(getDatabase());
            q2->prepare("INSERT INTO TestResults (modelID, testID, resultCode, terminalOutput) VALUES (?,?,?,?)");
            q2->addBindValue(modelID);
            q2->addBindValue(testID);
            q2->addBindValue("TODO: add result code");
            q2->addBindValue(*terminalOutput);
            dbExec(q2);   
        }

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
    if (!getDatabase().tables().contains("TestSuites"))
        dbExec("CREATE TABLE TestSuites (id INTEGER PRIMARY KEY, suiteName TEXT NOT NULL, UNIQUE(suiteName))");
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
			
            q->prepare("INSERT OR IGNORE INTO TestSuites VALUES (NULL, ?)");
            q->addBindValue(defaultTests[i].suiteName);
            dbExec(q);
			
			q->exec(QString("SELECT id FROM TestSuites WHERE suiteName = '%1'").arg(defaultTests[i].suiteName));
			QString testSuiteID;
			while (q->next()){
				testSuiteID = q->value(0).toString();
			}

            q->prepare("INSERT INTO TestsInSuite (testID, testSuiteID) VALUES (?, ?)");
            q->addBindValue(testID);
            q->addBindValue(testSuiteID);
            dbExec(q);
        }
    }
}

void VerificationValidationWidget::updateUI() {
	std::cout << "Updating UI" << std::endl;
}

void VerificationValidationWidget::setupUI() {
	updateUI();
	// Branch testDialog
	std::cout << "Branch: testDialog" << std::endl;
	
    // setup result table's column headers
    QStringList columnLabels;
    columnLabels << "   " << "   " << "Test Name" << "Description" << "Object Path";
    resultTable->setColumnCount(columnLabels.size());
    resultTable->setHorizontalHeaderLabels(columnLabels);
    resultTable->verticalHeader()->setVisible(false);
    resultTable->horizontalHeader()->setStretchLastSection(true);
    resultTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    addWidget(resultTable);

    // Get test list from db
    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);
    query.exec("Select testName from Tests ORDER BY id ASC");
    QStringList tests;
    while(query.next()){
    	tests << query.value(0).toString();
    }
    // Insert test list into tests checklist widget
    testList->addItems(tests);
    QListWidgetItem* item = 0;
    for (int i = 0; i < testList->count(); i++) {
        item = testList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    // Tests checklist add to dialog
   	testList->setMinimumWidth(testList->sizeHintForColumn(0)+40);
    
    // Get suite list from db
    query.exec("Select suiteName from TestSuites ORDER by id ASC");
    QStringList  testSuites;
    while(query.next()){
    	testSuites << query.value(0).toString();
    }
    // Insert suite list into suites checklist widget
    suiteList->addItems(testSuites);
    for (int i = 0; i < suiteList->count(); i++) {
        item = suiteList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    
    // Select ALL Suites
   	QListWidgetItem* suite_sa_item = new QListWidgetItem("Select All Suites");
   	suite_sa_item->setFlags(suite_sa_item->flags() | Qt::ItemIsUserCheckable);
   	suite_sa_item->setCheckState(Qt::Unchecked);
   	suite_sa->addItem(suite_sa_item);
   	suite_sa->setFixedHeight(20); // Hard coded list height --> change(?
   	
   	// Select ALL Tests
   	QListWidgetItem* test_sa_item = new QListWidgetItem("Select All Tests");
   	test_sa_item->setFlags(test_sa_item->flags() | Qt::ItemIsUserCheckable);
   	test_sa_item->setCheckState(Qt::Unchecked);
   	test_sa->addItem(test_sa_item);
   	test_sa->setFixedHeight(20); // Hard coded list height --> change(?
   	
   	// Popuulate Search bar
    /*
    QHBoxLayout *searchBar = new QHBoxLayout();
    QLineEdit *searchBox = new QLineEdit("");
    QPushButton *btn = new QPushButton("Search");
    searchBar->addWidget(searchBox);
    searchBar->addWidget(btn);
    
    dialog->addLayout(searchBar);
    selectTestsDialog->setLayout(dialog);
    
   	dialog->addSpacing(15);
   	
   	// Populate Test suite dropdown menu
   	QComboBox *testSuiteMenu = new QComboBox();
   	QStringList menu;
   	menu << "Test Suite 1" << "Test Suite 2" << "Test Suite 3" << "Test Suite 4";
   	testSuiteMenu->addItems(menu);
   	testSuiteMenu->setStyleSheet("border: 1px solid black");
   	selectTestsDialog->layout()->addWidget(testSuiteMenu);
   	
   	dialog->addSpacing(15);
   	*/
	
    // format and populate Select Tests dialog box
    selectTestsDialog->setModal(true);
    selectTestsDialog->setWindowTitle("Select Tests");
    QGridLayout* grid = new QGridLayout();
	
    QGroupBox* groupbox1 = new QGroupBox("Select Test Categories");
    QVBoxLayout* l_vbox = new QVBoxLayout();
    l_vbox->addWidget(suite_sa);
    l_vbox->addSpacing(10);
    l_vbox->addWidget(suiteList);
    groupbox1->setLayout(l_vbox);
    
    QGroupBox* groupbox2 = new QGroupBox("Test List");
    QVBoxLayout* r_vbox = new QVBoxLayout();
    r_vbox->addWidget(test_sa);
    r_vbox->addSpacing(10);
    r_vbox->addWidget(testList);
    groupbox2->setLayout(r_vbox);
    
    QGroupBox* groupbox3 = new QGroupBox();
    QDialogButtonBox* buttonOptions = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->addWidget(buttonOptions);
    groupbox3->setLayout(hbox);
    
    grid->addWidget(groupbox1, 0, 0);
    grid->addWidget(groupbox2, 0, 1);
    grid->addWidget(groupbox3, 1, 0, 1, 2);
    selectTestsDialog->setLayout(grid);
	
    connect(suite_sa, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(updateUI()));
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
