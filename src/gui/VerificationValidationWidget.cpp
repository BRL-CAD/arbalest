#include "VerificationValidationWidget.h"
#include <Document.h>
#include "MainWindow.h"
using Result = VerificationValidation::Result;
using DefaultTests = VerificationValidation::DefaultTests;
using Parser = VerificationValidation::Parser;

#define SHOW_ERROR_POPUP true

// TODO: if checksum doesn't match current test file, notify user

VerificationValidationWidget::VerificationValidationWidget(MainWindow* mainWindow, Document* document, QWidget* parent) : 
document(document), testList(new QListWidget()), resultTable(new QTableWidget()), 
selectTestsDialog(new QDialog()), statusBar(nullptr), mainWindow(mainWindow),
suiteList(new QListWidget()), test_sa(new QListWidget()), suite_sa(new  QListWidget()),
parentDockable(mainWindow->getVerificationValidationDockable()), msgBoxRes(NO_SELECTION) {
    QString dbName = "untitled" + QString::number(document->getDocumentId()) + ".atr";
    try { dbConnect(dbName); } catch (const std::runtime_error& e) { throw e; }
    dbInitTables();
    dbPopulateDefaults();
    setupUI();

    if (msgBoxRes == OPEN) {
        showAllResults();
        msgBoxRes = NO_SELECTION;
    } else if (msgBoxRes == DISCARD) {
        dbClearResults();
    }
}

VerificationValidationWidget::~VerificationValidationWidget() {
    std::cout << "closing " << dbConnectionName.toStdString() << std::endl;
    getDatabase().close();
    QSqlDatabase::removeDatabase(dbConnectionName);
}


void VerificationValidationWidget::showSelectTests() {
    statusBar->showMessage("Select tests to run...");
    selectTestsDialog->exec();
    //connect(selectTestsDialog, SIGNAL(accepted()), this, SLOT(runTests()));
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
    //clear result table
    resultTable->setRowCount(0);

    // Get list of checked tests
    QList<QListWidgetItem *> selected_tests;
    QListWidgetItem* item = 0;
    for (int i = 0; i < testList->count(); i++) {
		item = testList->item(i);
        if(item->checkState()){
            selected_tests.push_back(item);
        }
    }

    // Run tests
    int totalTests = selected_tests.count();
    if(totalTests ==  0){
        return;
    }

    QString status = "Finished running %1 / %2 tests";
    for(int i = 0; i < totalTests; i++){
        statusBar->showMessage(status.arg(i+1).arg(totalTests));
        int testID = testList->row(selected_tests[i]) + 1;
        QString testCommand = selected_tests[i]->toolTip();
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
    }
}

void VerificationValidationWidget::openATRFile() {
    // TODO: if .g not open, file opening will crash
    const QString filePath = QFileDialog::getOpenFileName(mainWindow->getDocumentArea(), tr("Open Arbalest Test Results"), QString(), "Arbalest Test Results (*.atr)");
    if (!filePath.isEmpty()) {
        try { dbConnect(filePath); } 
        catch (const std::runtime_error& e) { popup(e.what()); }
        catch (...) { popup("Failed to create a database connection for the .atr file"); }
        // TODO: open associated .g file; if doesn't exist, ask user to find it
        // TODO: load results into table
        // TODO: put more thought into how you want user to open and stuff
        // TODO: think about errors from opening multiple times without closing; also if already open
    }
}

void VerificationValidationWidget::dbConnect(const QString dbName) {
    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
        throw std::runtime_error("[Verification & Validation] ERROR: sqlite is not available");

    this->dbName = dbName;
    QString* fp = document->getFilePath();
    if (fp) this->dbName = fp->split("/").last() + ".atr";

    dbConnectionName = this->dbName + "-connection";
    QSqlDatabase db = QSqlDatabase::database(dbConnectionName, false);

    // if SQL connection already open, just switch to that tab
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
        throw std::runtime_error("[Verification & Validation] Document already open");
    }

    // if file exists, prompt before overwriting
    if (QFile::exists(this->dbName)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Detected existing test results in " + this->dbName + ".\nDo you want to open or discard the results?");
        msgBox.setInformativeText("Changes cannot be reverted.");
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
        QPushButton* discardButton = msgBox.addButton("Discard", QMessageBox::DestructiveRole);
        discardButton->setIcon(QIcon(":/icons/warning.png"));
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int res = msgBox.exec();
        if (res == QMessageBox::Open) {
            msgBoxRes = OPEN;
            parentDockable->setVisible(true);
        }
        else if (msgBox.clickedButton() == discardButton) { 
            msgBoxRes = DISCARD;
        }
        else {
            msgBoxRes = CANCEL;
            throw std::runtime_error("No changes were made.");
        }
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
        // dbExec("CREATE TABLE Tests (id INTEGER PRIMARY KEY, testName TEXT NOT NULL, testCommand TEXT NOT NULL UNIQUE, hasValArgs BOOL NOT NULL, Category TEXT NOT NULL)");
        dbExec("CREATE TABLE Tests (id INTEGER PRIMARY KEY, testName TEXT NOT NULL, testCommand TEXT NOT NULL UNIQUE)");
    if (!getDatabase().tables().contains("TestResults"))
        dbExec("CREATE TABLE TestResults (id INTEGER PRIMARY KEY, modelID INTEGER NOT NULL, testID INTEGER NOT NULL, resultCode TEXT, terminalOutput TEXT)");
    if (!getDatabase().tables().contains("Issues"))
        dbExec("CREATE TABLE Issues (id INTEGER PRIMARY KEY, testResultID INTEGER NOT NULL, objectIssueID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("ObjectIssue"))
        dbExec("CREATE TABLE ObjectIssue (id INTEGER PRIMARY KEY, objectName TEXT NOT NULL, issueDescription TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestSuites"))
        dbExec("CREATE TABLE TestSuites (id INTEGER PRIMARY KEY, suiteName TEXT NOT NULL, UNIQUE(suiteName))");
    if (!getDatabase().tables().contains("TestsInSuite"))
        dbExec("CREATE TABLE TestsInSuite (id INTEGER PRIMARY KEY, testSuiteID INTEGER NOT NULL, testID INTEGER NOT NULL)");
    // if (!getDatabase().tables().contains("TestArgs"))
    //     dbExec("CREATE TABLE TestArg (id INTEGER PRIMARY KEY, testID INTEGER NOT NULL, argIdx INTEGER NOT NULL, arg TEXT NOT NULL, isVarArg BOOL NOT NULL, defaultVal TEXT)");
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
			
            q->prepare("INSERT OR IGNORE INTO TestSuites VALUES (NULL, ?)");
            q->addBindValue(DefaultTests::allTests[i].suiteName);
            dbExec(q);
			
			q->prepare("SELECT id FROM TestSuites WHERE suiteName = ?");
            q->addBindValue(DefaultTests::allTests[i].suiteName);
            dbExec(q);
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

void VerificationValidationWidget::searchTests(const QString &input)  {
    QList<QListWidgetItem *> tests = testList->findItems(input, Qt::MatchContains);
    QListWidgetItem* item = 0;
    for (int i = 0; i < testList->count(); i++) {
		item = testList->item(i);
        if(!tests.contains(item))
            item->setHidden(true);
        else
            item->setHidden(false);
    }
}

void VerificationValidationWidget::updateSuiteSelectAll(QListWidgetItem* sa_option) {
    QListWidgetItem* item = 0;
    for (int i = 0; i < suiteList->count(); i++) {
		item = suiteList->item(i);
		if(sa_option->checkState()){
			item->setCheckState(Qt::Checked);
		} else {
			item->setCheckState(Qt::Unchecked);
		}
        updateTestListWidget(item);
	}
}

void VerificationValidationWidget::updateTestSelectAll(QListWidgetItem* sa_option) {
	QListWidgetItem* item = 0;
    for (int i = 0; i < testList->count(); i++) {
		item = testList->item(i);
		if(sa_option->checkState()){
			item->setCheckState(Qt::Checked);
		} else {
			item->setCheckState(Qt::Unchecked);
		}
	}

    if(sa_option->checkState()){
		suite_sa->item(0)->setCheckState(Qt::Checked);
	} else {
        suite_sa->item(0)->setCheckState(Qt::Unchecked);
    }
    
    updateSuiteSelectAll(suite_sa->item(0));
}

void VerificationValidationWidget::checkSuiteSA() {
    QListWidgetItem* item = 0;
    for (int i = 0; i < suiteList->count(); i++) {
        item = suiteList->item(i);
        if(!item->checkState()){
            return;
        }
    }
    suite_sa->item(0)->setCheckState(Qt::Checked);
}

void VerificationValidationWidget::checkTestSA() {
    // Check if all checked
    QListWidgetItem* item = 0;
    for (int i = 0; i < testList->count(); i++) {
        item = testList->item(i);
        if(!item->checkState()){
            return;
        }
    }
    test_sa->item(0)->setCheckState(Qt::Checked);
}

void VerificationValidationWidget::updateTestListWidget(QListWidgetItem* suite_clicked) {
    QSqlQuery* q = new QSqlQuery(getDatabase());
    q->prepare("Select testID from TestsInSuite Where testSuiteID = (SELECT id FROM TestSuites WHERE suiteName = :suiteName)");
    q->bindValue(":suiteName", suite_clicked->text());
    dbExec(q, !SHOW_ERROR_POPUP);

    QListWidgetItem* item = 0;
    while(q->next()){
        int row = q->value(0).toInt() - 1;
        item = testList->item(row);
        if(suite_clicked->checkState()){
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        testListSelection(item);
    }

    if(!suite_clicked->checkState()){
        suite_sa->item(0)->setCheckState(Qt::Unchecked);
    }
    checkSuiteSA();
}

void VerificationValidationWidget::testListSelection(QListWidgetItem* test_clicked) {
    QSqlQuery* q1 = new QSqlQuery(getDatabase());
    QSqlQuery* q2 = new QSqlQuery(getDatabase());
    
    q1->prepare("Select testSuiteID from TestsInSuite Where testID = (SELECT id FROM Tests WHERE testName = :testName)");
    q1->bindValue(":testName", test_clicked->text());
    dbExec(q1, !SHOW_ERROR_POPUP);
    while(q1->next()){
        QListWidgetItem* suite = suiteList->item(q1->value(0).toInt()-1);
        if(!test_clicked->checkState()){
            // If any test unchekced -> update test sa and suite containing test_clicked
            if(suite->checkState()){
                suite->setCheckState(Qt::Unchecked);
                suite_sa->item(0)->setCheckState(Qt::Unchecked);
            }
        } else {
            // Check if all test in a suite is checked  -> check suite
            q2->prepare("Select testID from TestsInSuite Where testSuiteID = :suiteID");
            q2->bindValue(":suiteID", q1->value(0).toInt());
            dbExec(q2, !SHOW_ERROR_POPUP);
            while(q2->next()){
                QListWidgetItem* test = testList->item(q2->value(0).toInt()-1);
                if(!test->checkState()){
                    return;
                }
            }
            suite->setCheckState(Qt::Checked);
        }
    }

    if(!test_clicked->checkState()){
        test_sa->item(0)->setCheckState(Qt::Unchecked);
    }
    checkSuiteSA();
    checkTestSA();
}

void VerificationValidationWidget::userInputDialogUI(QListWidgetItem* test) {
    QDialog* userInputDialog = new QDialog();
    userInputDialog->setModal(true);
    userInputDialog->setWindowTitle("Custom Inputs");

    QVBoxLayout* vLayout = new QVBoxLayout();
    QFormLayout* formLayout = new QFormLayout();

    vLayout->addWidget(new QLabel(test->text()));

    QLineEdit* input1 = new QLineEdit();
    QLineEdit* input2 = new QLineEdit();
    
    formLayout->addRow(QString("Input 1:  "), input1);
    formLayout->addRow(QString("Input 1:  "), input2);
    
    vLayout->addLayout(formLayout);
    
    QPushButton* setBtn = new  QPushButton("Set");
    
    vLayout->addWidget(setBtn);
    userInputDialog->setLayout(vLayout);

    userInputDialog->exec();
}

void VerificationValidationWidget::setupUI() {
    // TODO: allow input
    // TODO: select tops
    // TODO: add test categories in test lists
	
    // setup result table's column headers
    QStringList columnLabels;
    columnLabels << "   " << "Test Name" << "Description" << "Object Path";
    resultTable->setColumnCount(columnLabels.size());
    resultTable->setHorizontalHeaderLabels(columnLabels);
    resultTable->verticalHeader()->setVisible(false);
    resultTable->horizontalHeader()->setStretchLastSection(true);
    resultTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    addWidget(resultTable);

    // Get test list from db
    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);
    query.exec("Select testName, testCommand from Tests ORDER BY id ASC");
    QStringList tests;
    QStringList testCmds;
    while(query.next()){
    	tests << query.value(0).toString();
        testCmds << query.value(1).toString();
    }

    // Insert test list into tests checklist widget
    testList->addItems(tests);
    QListWidgetItem* item = 0;
    QIcon edit_icon(":/icons/editIcon.png");
    for (int i = 0; i < testList->count(); i++) {
        item = testList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setToolTip(testCmds[i]);

        // TODO: change to gqa cmd only
        item->setIcon(edit_icon);
        // item->setFlags(item->flags() &  ~Qt::ItemIsSelectable);
    }

    // Tests checklist add to dialog
   	testList->setMinimumWidth(testList->sizeHintForColumn(0)+40);
    
    // Get suite list from db
    query.exec("Select suiteName from TestSuites ORDER by id ASC");
    QStringList testSuites;
    while(query.next()){
    	testSuites << query.value(0).toString();
    }
    // Insert suite list into suites checklist widget
    suiteList->addItems(testSuites);
    
    for (int i = 0; i < suiteList->count(); i++) {
        item = suiteList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setFlags(item->flags() &  ~Qt::ItemIsSelectable);
    }
    
    // Select ALL Suites
   	QListWidgetItem* suite_sa_item = new QListWidgetItem("Select All Suites");
   	suite_sa_item->setFlags(suite_sa_item->flags() | Qt::ItemIsUserCheckable);
   	suite_sa_item->setCheckState(Qt::Unchecked);
   	suite_sa->addItem(suite_sa_item);
   	suite_sa->setFixedHeight(20);
    suite_sa_item->setFlags(suite_sa_item->flags() &  ~Qt::ItemIsSelectable);
   	
   	// Select ALL Tests
   	QListWidgetItem* test_sa_item = new QListWidgetItem("Select All Tests");
   	test_sa_item->setFlags(test_sa_item->flags() | Qt::ItemIsUserCheckable);
   	test_sa_item->setCheckState(Qt::Unchecked);
   	test_sa->addItem(test_sa_item);
   	test_sa->setFixedHeight(20);
   	test_sa_item->setFlags(test_sa_item->flags() &  ~Qt::ItemIsSelectable);

   	// Popuulate Search bar
    QHBoxLayout* searchBar = new QHBoxLayout();
    QLabel* searchLabel = new QLabel("Search: ");
    searchBox = new QLineEdit("");
    searchBar->addWidget(searchLabel);
    searchBar->addWidget(searchBox);
	
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
    r_vbox->addLayout(searchBar);
    r_vbox->addSpacing(5);
    r_vbox->addWidget(test_sa);
    r_vbox->addSpacing(5);
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
	
    // Select all signal connect function
    connect(suite_sa, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(updateSuiteSelectAll(QListWidgetItem *)));
    connect(test_sa, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(updateTestSelectAll(QListWidgetItem *)));
    
    // Suite select signal connect function
    connect(suiteList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(updateTestListWidget(QListWidgetItem *)));
    // Test select signal connect function
    connect(testList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(testListSelection(QListWidgetItem*)));

    // Search button pressed signal select function
    connect(searchBox, SIGNAL(textEdited(const QString &)), this, SLOT(searchTests(const QString &)));

    // Test input for gqa
    connect(testList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(userInputDialogUI(QListWidgetItem *)));
    
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

void VerificationValidationWidget::dbClearResults() {
    dbExec("DELETE FROM TestResults");
    dbExec("DELETE FROM Issues");
    dbExec("DELETE FROM ObjectIssue");
}

void VerificationValidationWidget::resizeEvent(QResizeEvent* event) {
    resultTable->setColumnWidth(0, this->width() * 0.025);
    resultTable->setColumnWidth(1, this->width() * 0.125);
    resultTable->setColumnWidth(2, this->width() * 0.60);
    resultTable->setColumnWidth(3, this->width() * 0.25);

    QHBoxWidget::resizeEvent(event);
}

void VerificationValidationWidget::setupDetailedResult(int row, int column) {
    // QDialog* result_dialog = new QDialog();
    // result_dialog->exec();
    // result_dialog->setModal(true);
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

        // Change to hide icon image path from showing
        QTableWidgetItem* icon_item = new QTableWidgetItem;
        QIcon icon(iconPath);
        icon_item->setIcon(icon);
        resultTable->setItem(resultTable->rowCount()-1, RESULT_CODE_COLUMN, icon_item);

        resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem(testName));
        resultTable->setItem(resultTable->rowCount()-1, DESCRIPTION_COLUMN, new QTableWidgetItem(issueDescription));
        resultTable->setItem(resultTable->rowCount()-1, OBJPATH_COLUMN, new QTableWidgetItem(objectName));

        // Only select rows, disable edit
        resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        // Double click event signal trigger
        //connect(resultTable, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(setupDetailedResult()));
        connect(resultTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(setupDetailedResult(int, int)));
    }
}

void VerificationValidationWidget::showAllResults() {
    QSqlQuery* q = new QSqlQuery(getDatabase());
    q->prepare("SELECT id FROM TestResults WHERE modelID = ?");
    q->addBindValue(modelID);
    dbExec(q);

    QString testResultID;
    while (q && q->next()) {
        testResultID = q->value(0).toString();
        showResult(testResultID);
    }
}
