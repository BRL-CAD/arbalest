#include "VerificationValidationWidget.h"
#include <Document.h>
#include "MainWindow.h"
using Result = VerificationValidation::Result;
using DefaultTests = VerificationValidation::DefaultTests;
using Parser = VerificationValidation::Parser;

#define SHOW_ERROR_POPUP true

// TODO: if checksum doesn't match current test file, notify user

VerificationValidationWidget::VerificationValidationWidget(MainWindow* mainWindow, Document* document, QWidget* parent) : 
document(document), statusBar(nullptr), mainWindow(mainWindow), parentDockable(mainWindow->getVerificationValidationDockable()),
testList(new QListWidget()), resultTable(new QTableWidget()), selectTestsDialog(new QDialog()),
suiteList(new QListWidget()), test_sa(new QListWidget()), suite_sa(new QListWidget()),
msgBoxRes(NO_SELECTION), folderName("atr"), dbConnectionName(""),
dbFilePath(folderName + "/untitled" + QString::number(document->getDocumentId()) + ".atr")
{
    if (!dbConnectionName.isEmpty()) return;
    if (!QDir(folderName).exists() && !QDir().mkdir(folderName)) popup("Failed to create " + folderName + " folder");
    
    try { dbConnect(dbFilePath); } catch (const std::runtime_error& e) { throw e; }
    dbInitTables();
    dbPopulateDefaults();
    setupUI();

    updateDockableHeader();
    validateChecksum();
    if (msgBoxRes == OPEN) {
        showAllResults();
        msgBoxRes = NO_SELECTION;
    } else if (msgBoxRes == DISCARD) {
        dbClearResults();
        resultTable->setRowCount(0);
    }
}

VerificationValidationWidget::~VerificationValidationWidget() {
    QString dockableTitle = "Verification & Validation";
    QLabel *title = new QLabel(dockableTitle);
    title->setObjectName("dockableHeader");
    parentDockable->setTitleBarWidget(title);
    dbClose();
}

void VerificationValidationWidget::showSelectTests() {
    emit mainWindow->setStatusBarMessage("Select tests to run...");
    selectTestsDialog->exec();
}

QString* VerificationValidationWidget::runTest(const QString& cmd) {
    QString filepath = *(document->getFilePath());
    struct ged* dbp = mgedRun(cmd, filepath);
    QString* result = new QString(bu_vls_addr(dbp->ged_result_str));
    ged_close(dbp);

    return result;
}

void VerificationValidationWidget::runTests() {
    validateChecksum();
    dbUpdateModelUUID();
    dbClearResults();
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

    for(int i = 0; i < totalTests; i++){
        emit mainWindow->setStatusBarMessage(i+1, totalTests);
        QSqlQuery* tmp = new QSqlQuery(getDatabase());
        tmp->prepare("SELECT id FROM Tests WHERE testName = ?");
        tmp->addBindValue(selected_tests[i]->text());
        dbExec(tmp);

        if (!tmp->next()) continue;

        QString testID = tmp->value(0).toString();
        QString testCommand = selected_tests[i]->toolTip();
        const QString* terminalOutput = runTest(testCommand);
        
        QString executableName = testCommand.split(' ').first();
        Result* result = nullptr;
        // find proper parser
        if (QString::compare(executableName, "search", Qt::CaseInsensitive) == 0)
            result = Parser::search(testCommand, terminalOutput);
        else if (QString::compare(executableName, "gqa", Qt::CaseInsensitive) == 0)
            result = Parser::gqa(testCommand, terminalOutput);
        else if (QString::compare(executableName, "title", Qt::CaseInsensitive) == 0)
            result = Parser::title(testCommand, terminalOutput);

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

void VerificationValidationWidget::dbConnect(const QString dbFilePath) {
    if (!QSqlDatabase::isDriverAvailable("QSQLITE"))
        throw std::runtime_error("[Verification & Validation] ERROR: sqlite is not available");

    this->dbName = dbFilePath;
    QString* fp = document->getFilePath();
    if (fp) {
        QStringList fpList = fp->split("/");
        this->dbName = folderName + "/" + fpList.last() + ".atr";
        this->dbFilePath = QDir(this->dbName).absolutePath();
    }

    dbConnectionName = this->dbName + "-connection";
    QSqlDatabase db = getDatabase();

    // if SQL connection already open, just switch to that tab
    if (dbIsAlive(db)) {
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
        throw std::runtime_error("");
    }

    // if file exists, prompt before overwriting
    if (QFile::exists(this->dbName)) {
        QMessageBox msgBox; 
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Detected existing test results in " + this->dbName + ".\n\nDo you want to open or discard the results?");
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
        delete dbExec("CREATE TABLE Model (id INTEGER PRIMARY KEY, filepath TEXT NOT NULL UNIQUE, uuid TEXT NOT NULL)");
    if (!getDatabase().tables().contains("Tests"))
        delete dbExec("CREATE TABLE Tests (id INTEGER PRIMARY KEY, testName TEXT NOT NULL, testCommand TEXT NOT NULL, hasValArgs BOOL NOT NULL, category TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestResults"))
        delete dbExec("CREATE TABLE TestResults (id INTEGER PRIMARY KEY, modelID INTEGER NOT NULL, testID INTEGER NOT NULL, resultCode TEXT, terminalOutput TEXT)");
    if (!getDatabase().tables().contains("Issues"))
        delete dbExec("CREATE TABLE Issues (id INTEGER PRIMARY KEY, testResultID INTEGER NOT NULL, objectIssueID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("ObjectIssue"))
        delete dbExec("CREATE TABLE ObjectIssue (id INTEGER PRIMARY KEY, objectName TEXT NOT NULL, issueDescription TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestSuites"))
        delete dbExec("CREATE TABLE TestSuites (id INTEGER PRIMARY KEY, suiteName TEXT NOT NULL, UNIQUE(suiteName))");
    if (!getDatabase().tables().contains("TestsInSuite"))
        delete dbExec("CREATE TABLE TestsInSuite (id INTEGER PRIMARY KEY, testSuiteID INTEGER NOT NULL, testID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("TestArg"))
        delete dbExec("CREATE TABLE TestArg (id INTEGER PRIMARY KEY, testID INTEGER NOT NULL, argIdx INTEGER NOT NULL, arg TEXT NOT NULL, isVarArg BOOL NOT NULL, defaultVal TEXT)");
}

void VerificationValidationWidget::dbPopulateDefaults() {
    QSqlQuery* q;
    QString gFilePath = *document->getFilePath();
    QString* uuid = generateUUID(gFilePath);

    if (!uuid) throw std::runtime_error("Failed to generate UUID for " + gFilePath.toStdString());

    // if Model table empty, assume new db and insert model info
    q = new QSqlQuery(getDatabase());
    q->prepare("SELECT id FROM Model WHERE filepath=?");
    q->addBindValue(QDir(*document->getFilePath()).absolutePath());
    dbExec(q, !SHOW_ERROR_POPUP);

    if (!q->next()) {
        q->prepare("INSERT INTO Model (filepath, uuid) VALUES (?, ?)");
        q->addBindValue(QDir(*document->getFilePath()).absolutePath());
        q->addBindValue(*uuid);
        dbExec(q);
        modelID = q->lastInsertId().toString();
    } else {
        modelID = q->value(0).toString();
    }

    delete q;

    // if Tests table empty, new db and insert tests
    // note: this doesn't repopulate deleted tests, unless all tests deleted
    q = dbExec("SELECT id FROM Tests", !SHOW_ERROR_POPUP);
    if (!q->next()) {
        for (int i = 0; i < DefaultTests::allTests.size(); i++) {
            q->prepare("INSERT INTO Tests (testName, testCommand, hasValArgs, category) VALUES (:testName, :testCommand, :hasValArgs, :category)");
            q->bindValue(":testName", DefaultTests::allTests[i].testName);
            q->bindValue(":testCommand", DefaultTests::allTests[i].testCommand);
            q->bindValue(":hasValArgs", DefaultTests::allTests[i].hasVariable);
            q->bindValue(":category", DefaultTests::allTests[i].category);
            dbExec(q);

            QString testID = q->lastInsertId().toString();
			
            q->prepare("INSERT OR IGNORE INTO TestSuites VALUES (NULL, ?)");
            q->addBindValue(DefaultTests::allTests[i].suiteName);
            dbExec(q);

            for (int j = 0; j < DefaultTests::allTests[i].ArgList.size(); j++){
                q->prepare("INSERT INTO TestArg (testID, argIdx, arg, isVarArg, defaultVal) VALUES (:testID, :argIdx, :arg, :isVarArg, :defaultVal)");
                q->bindValue(":testID", testID);
                q->bindValue(":argIdx", j+1);
                q->bindValue(":arg", DefaultTests::allTests[i].ArgList[j].argument);
                q->bindValue(":isVarArg", DefaultTests::allTests[i].ArgList[j].isVariable);
                q->bindValue(":defaultVal", DefaultTests::allTests[i].ArgList[j].defaultValue);
                dbExec(q);
            } 
			
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
    delete q;
}

void VerificationValidationWidget::searchTests(const QString &input)  {
    // Hide category when search
    if(input.isEmpty()){
        QListWidgetItem* item = 0;
        for (int i = 0; i < testList->count(); i++) {
            item = testList->item(i);
            item->setHidden(false);
        }
    } else {
        QList<QListWidgetItem *> tests = testList->findItems(input, Qt::MatchContains);
        QListWidgetItem* item = 0;
        for (int i = 0; i < testList->count(); i++) {
            item = testList->item(i);
            if(!tests.contains(item) || item->toolTip() == "Category")
                item->setHidden(true);
            else
                item->setHidden(false);
        }
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
    for (int i = 0; i < itemToTestMap.size(); i++) {
        auto it = itemToTestMap.begin();
        std::advance(it, i);
        item = it->first;
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
    QListWidgetItem* item = 0;
    for (int i = 0; i < itemToTestMap.size(); i++) {
        auto it = itemToTestMap.begin();
        std::advance(it, i);
        item = it->first;
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
    dbExec(q);

    QListWidgetItem* item = 0;
    while(q->next()){
        int id = q->value(0).toInt();
        item = idToItemMap.at(id);

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
    delete q;
}

void VerificationValidationWidget::testListSelection(QListWidgetItem* test_clicked) {
    if(test_clicked->toolTip() == "Category"){
        return;
    }
    QSqlQuery* q1 = new QSqlQuery(getDatabase());
    QSqlQuery* q2 = new QSqlQuery(getDatabase());
    
    q1->prepare("Select testSuiteID from TestsInSuite Where testID = :id");
    q1->bindValue(":id", itemToTestMap.at(test_clicked).first);
    dbExec(q1);
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
            dbExec(q2);
            while(q2->next()){
                QListWidgetItem* test = idToItemMap.at(q2->value(0).toInt());

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

    delete q1;
    delete q2;
}

void VerificationValidationWidget::userInputDialogUI(QListWidgetItem* test) {
    if(test->toolTip() !=  "Category"){
        if(itemToTestMap.at(test).second.hasVariable) {
            QDialog* userInputDialog = new QDialog();
            userInputDialog->setModal(true);
            userInputDialog->setWindowTitle("Custom Argument Value");

            QVBoxLayout* vLayout = new QVBoxLayout();
            QFormLayout* formLayout = new QFormLayout();

            vLayout->addWidget(new QLabel("Test Name: "+ test->text()));
            vLayout->addSpacing(5);
            vLayout->addWidget(new QLabel("Test Command: "+ itemToTestMap.at(test).second.getCmdWithArgs()));
            vLayout->addSpacing(15);

            std::vector<QLineEdit*> input_vec;
            for(int i = 0; i < itemToTestMap.at(test).second.ArgList.size();  i++){
                if(itemToTestMap.at(test).second.ArgList[i].isVariable){
                    input_vec.push_back(new QLineEdit(itemToTestMap.at(test).second.ArgList[i].defaultValue));
                    formLayout->addRow(itemToTestMap.at(test).second.ArgList[i].argument, input_vec.back());
                    formLayout->setSpacing(10);
                } else {
                    input_vec.push_back(NULL);
                }
            }
            
            vLayout->addLayout(formLayout);
            QPushButton* setBtn = new QPushButton("Set");
            vLayout->addWidget(setBtn);
            userInputDialog->setLayout(vLayout);

            connect(setBtn, &QPushButton::clicked, [this, test, input_vec](){
                for(int i = 0; i < itemToTestMap.at(test).second.ArgList.size();  i++){
                    if(itemToTestMap.at(test).second.ArgList[i].isVariable){
                        itemToTestMap.at(test).second.ArgList[i].updateValue(input_vec[i]->text());
                    }
                }

                test->setToolTip(itemToTestMap.at(test).second.getCmdWithArgs());
            });
            
            connect(setBtn, &QPushButton::clicked, userInputDialog, &QDialog::accept);
            userInputDialog->exec();
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

    // Get test list from db
    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);
    query.exec("Select id, testName, testCommand, hasValArgs, category from Tests ORDER BY category ASC");

    QStringList testIdList;
    QStringList tests;
    QStringList testCmds;
    QStringList hasVariableList;
    QStringList categoryList;

    while(query.next()){
        testIdList << query.value(0).toString();
    	tests << query.value(1).toString();
        testCmds << query.value(2).toString();
        hasVariableList << query.value(3).toString();
        categoryList << query.value(4).toString();
    }

    // Creat test widget item
    QIcon edit_icon(":/icons/editIcon.png");
    for (int i = 0; i < tests.size(); i++) {
        QListWidgetItem* item = new QListWidgetItem(tests[i]);
        int id = testIdList[i].toInt();
        bool hasValArgs = hasVariableList[i].toInt();

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setFlags(item->flags() &  ~Qt::ItemIsSelectable);
        if(hasValArgs) {
            item->setIcon(edit_icon);
        }
        std::vector<VerificationValidation::Arg> ArgList;
        query.prepare("Select arg, isVarArg, defaultVal FROM TestArg Where testID = :id ORDER BY argIdx");
        query.bindValue(":id", id);
        query.exec();
        while(query.next()){
            ArgList.push_back(VerificationValidation::Arg(query.value(0).toString(), query.value(1).toBool(), query.value(2).toString()));
        }
        itemToTestMap.insert(make_pair(item, make_pair(id, VerificationValidation::Test({tests[i], testCmds[i], NULL, categoryList[i], hasValArgs, ArgList}))));
        idToItemMap.insert(make_pair(id, item));
        item->setToolTip(itemToTestMap.at(item).second.getCmdWithArgs());
        testList->addItem(item);
    }

    // Add test categories in test lists
    int offset = 0;
    for (int i = 0; i < categoryList.size(); i++) {
        QList<QListWidgetItem *> items = testList->findItems(categoryList[i], Qt::MatchExactly);
        if (items.size() == 0) {
            QListWidgetItem* item = new QListWidgetItem(categoryList[i]);
            item->setFlags(item->flags() &  ~Qt::ItemIsSelectable);
            item->setToolTip("Category");
            QFont itemFont = item->font();
            itemFont.setWeight(QFont::Bold);
            item->setFont(itemFont);
            testList->insertItem(i+offset, item);
            offset += 1;
        }
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
    QListWidgetItem* item = 0;
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
    QLineEdit* searchBox = new QLineEdit("");
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
    hbox->addWidget(new QLabel("Warning: running tests will overwrite your current results."));
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
    // Run test & exit
    connect(buttonOptions, &QDialogButtonBox::accepted, selectTestsDialog, &QDialog::accept);
    connect(buttonOptions, &QDialogButtonBox::accepted, this, &VerificationValidationWidget::runTests);
    connect(buttonOptions, &QDialogButtonBox::rejected, selectTestsDialog, &QDialog::reject);
    // Open details dialog
    connect(resultTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(setupDetailedResult(int, int)));
    connect(resultTable, SIGNAL(cellClicked(int, int)), this, SLOT(visualizeOverlaps(int, int)));
}

QSqlQuery* VerificationValidationWidget::dbExec(QString command, bool showErrorPopup) {
    QSqlDatabase db = getDatabase();
    QSqlQuery* query = new QSqlQuery(command, db);
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
    delete dbExec("DELETE FROM TestResults");
    delete dbExec("DELETE FROM Issues");
    delete dbExec("DELETE FROM ObjectIssue");
}

void VerificationValidationWidget::resizeEvent(QResizeEvent* event) {
    resultTable->setColumnWidth(0, this->width() * 0.025);
    resultTable->setColumnWidth(1, this->width() * 0.125);
    resultTable->setColumnWidth(2, this->width() * 0.60);
    resultTable->setColumnWidth(3, this->width() * 0.25);

    QHBoxWidget::resizeEvent(event);
}

void VerificationValidationWidget::setupDetailedResult(int row, int column) {
    QDialog* detail_dialog = new QDialog();
    detail_dialog->setModal(true);
    detail_dialog->setWindowTitle("Test Result Details");

    QVBoxLayout* detailLayout = new QVBoxLayout();

    QString resultCode;
    
    QTableWidgetItem* testNameItem = resultTable->item(row, TEST_NAME_COLUMN);
    QTableWidgetItem* descriptionItem = resultTable->item(row, DESCRIPTION_COLUMN);
    QTableWidgetItem* objPathItem = resultTable->item(row, OBJPATH_COLUMN);

    QString testName = (testNameItem) ? testNameItem->text() : "";
    QString description = (descriptionItem) ? descriptionItem->text() : "";
    QString objPath = (objPathItem) ? objPathItem->text() : "";
    QSqlQuery* q = new QSqlQuery(getDatabase());
    q->prepare("SELECT id FROM Tests WHERE testName = ?");
    q->addBindValue(testName);
    dbExec(q);
    if (!q->next()) {
        popup("Failed to show testName: " + testName);
        return;
    }

    int testID = q->value(0).toInt();
    QString testCommand = itemToTestMap.at(idToItemMap.at(testID)).second.getCmdWithArgs();

    QSqlQuery* q2 = new QSqlQuery(getDatabase());
    q2->prepare("SELECT terminalOutput, resultCode FROM TestResults WHERE testID = ?");
    q2->addBindValue(testID);
    dbExec(q2);
    if (!q2->next()) {
        popup("Failed to show testID: " + testID);
        return;
    }

    QString terminalOutput = q2->value(0).toString();
    int code = q2->value(1).toInt();
    if(code == Result::Code::PASSED)
        resultCode = "Passed";
    else if(code == Result::Code::WARNING)
        resultCode = "Warning";
    else if(code == Result::Code::FAILED)
        resultCode = "Failed";
    else
        resultCode = "Unparseable";

    QLabel *testNameHeader = new QLabel("Test Name:");
    testNameHeader->setStyleSheet("font-weight: bold");
    QLabel *commandHeader = new QLabel("Command:");
    commandHeader->setStyleSheet("font-weight: bold");
    QLabel *resultCodeHeader = new QLabel("Result Code:");
    resultCodeHeader->setStyleSheet("font-weight: bold");
    QLabel *descriptionHeader = new QLabel("Description:");
    descriptionHeader->setStyleSheet("font-weight: bold");
    QLabel *rawOutputHeader = new QLabel("Raw Output:");
    rawOutputHeader->setStyleSheet("font-weight: bold");

    detailLayout->addWidget(testNameHeader);
    detailLayout->addWidget(new QLabel(testName));
    detailLayout->addSpacing(10);
    detailLayout->addWidget(commandHeader);
    QLabel* testCmdLabel = new QLabel(testCommand);
    testCmdLabel->setFixedWidth(testCmdLabel->sizeHint().width()+120);
    detailLayout->addWidget(testCmdLabel);
    detailLayout->addSpacing(10);
    detailLayout->addWidget(resultCodeHeader);
    detailLayout->addWidget(new QLabel(resultCode));
    detailLayout->addSpacing(10);
    detailLayout->addWidget(descriptionHeader);
    detailLayout->addWidget(new QLabel(description));
    detailLayout->addSpacing(10);
    detailLayout->addWidget(rawOutputHeader);
    terminalOutput = "<div style=\"font-weight:500; color:#39ff14;\">arbalest> "+testCommand+"</div><br><div style=\"font-weight:500; color:white;\">"+terminalOutput+"</div>";
    QTextEdit* rawOutputBox = new QTextEdit("<html><pre>"+terminalOutput+"</pre></html>");
    rawOutputBox->setReadOnly(true);
    QPalette rawOutputBox_palette = rawOutputBox->palette();
    rawOutputBox_palette.setColor(QPalette::Base, Qt::black);
    rawOutputBox->setPalette(rawOutputBox_palette);
    detailLayout->addWidget(rawOutputBox);

    detail_dialog->setLayout(detailLayout);
    detail_dialog->exec();
}

void VerificationValidationWidget::visualizeOverlaps(int row, int column) {
    QString resultCode;
    
    QTableWidgetItem* testNameItem = resultTable->item(row, TEST_NAME_COLUMN);
    QTableWidgetItem* descriptionItem = resultTable->item(row, DESCRIPTION_COLUMN);
    QTableWidgetItem* objPathItem = resultTable->item(row, OBJPATH_COLUMN);

    QString testName = (testNameItem) ? testNameItem->text() : "";
    QString description = (descriptionItem) ? descriptionItem->text() : "";
    QString objPath = (objPathItem) ? objPathItem->text() : "";
    QSqlQuery* q = new QSqlQuery(getDatabase());
    q->prepare("SELECT id FROM Tests WHERE testName = ?");
    q->addBindValue(testName);
    dbExec(q);
    if (!q->next()) {
        popup("Failed to show testName: " + testName);
        return;
    }

    int testID = q->value(0).toInt();

    QSqlQuery* q2 = new QSqlQuery(getDatabase());
    q2->prepare("SELECT resultCode FROM TestResults WHERE testID = ?");
    q2->addBindValue(testID);
    dbExec(q2);
    if (!q2->next()) {
        popup("Failed to show testID: " + testID);
        return;
    }

    int code = q2->value(0).toInt();

    if((code == Result::Code::WARNING || code == Result::Code::FAILED) && (testName == DefaultTests::NO_OVERLAPS.testName || testName == DefaultTests::NO_NULL_REGIONS.testName)) {
        //QString visualizeCmd = "gqa -Ap -g"+DefaultTests::NO_OVERLAPS.ArgList[1].defaultValue+" -t"+DefaultTests::NO_OVERLAPS.ArgList[2].defaultValue+" "+objName1+" "+objName2;
        QStringList splitString = description.split('\'');
        QString objName1 = splitString[1];
        QString objName2 = objName1;
        if(testName == DefaultTests::NO_OVERLAPS.testName)
            objName2 = splitString[3];

        ObjectTree *objTree = document->getObjectTree();
        QHash<int, QString> nameMap = document->getObjectTree()->getNameMap();
        QHashIterator<int, QString> iter1(nameMap);
        QHashIterator<int, QString> iter2(nameMap);
        while(iter1.hasNext()) {
            iter1.next();
            objTree->changeVisibilityState(iter1.key(), false);
        }
        while(iter2.hasNext()) {
            iter2.next();
            if(iter2.value() == objName1 || iter2.value() == objName2)
                objTree->changeVisibilityState(iter2.key(), true);
        }
        document->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        document->getDisplayGrid()->forceRerenderAllDisplays();
        document->getObjectTreeWidget()->refreshItemTextColors();
    }
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

    QString iconPath = "";
    QString objectName;
    QString issueDescription;

    if (resultCode == Result::Code::PASSED) {
        resultTable->insertRow(resultTable->rowCount());
        iconPath = ":/icons/passed.png";
        resultTable->setItem(resultTable->rowCount()-1, RESULT_CODE_COLUMN, new QTableWidgetItem(QIcon(iconPath), ""));
        resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem(testName));
    } 

    else if (resultCode == Result::Code::UNPARSEABLE) {
        resultTable->insertRow(resultTable->rowCount());
        iconPath = ":/icons/unparseable.png";
        resultTable->setItem(resultTable->rowCount()-1, RESULT_CODE_COLUMN, new QTableWidgetItem(QIcon(iconPath), ""));
        resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem(testName));
    }

    else {
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

            objectName = q3->value(0).toString();
            issueDescription = q3->value(1).toString().replace("\n", "");

            resultTable->insertRow(resultTable->rowCount());

            if (resultCode == VerificationValidation::Result::Code::FAILED)
                iconPath = ":/icons/error.png";
            else if (resultCode == VerificationValidation::Result::Code::WARNING)
                iconPath = ":/icons/warning.png";                

            // Change to hide icon image path from showing
            resultTable->setItem(resultTable->rowCount()-1, RESULT_CODE_COLUMN, new QTableWidgetItem(QIcon(iconPath), ""));
            resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem(testName));
            resultTable->setItem(resultTable->rowCount()-1, DESCRIPTION_COLUMN, new QTableWidgetItem(issueDescription));
            resultTable->setItem(resultTable->rowCount()-1, OBJPATH_COLUMN, new QTableWidgetItem(objectName));

            delete q3;
        }
        delete q2;
    }
    
    // Only select rows, disable edit
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    delete q;
    qApp->processEvents();
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

    delete q;
}

void VerificationValidationWidget::validateChecksum() {
    QSqlQuery* q = dbExec("SELECT uuid FROM Model");
    if (!q->next()) { popup("Failed to validate checksum (failed get UUID from Model)"); return; }
    QString uuid = q->value(0).toString();
    delete q;

    QString gFilePath = *document->getFilePath();
    QString* gFileUUID = generateUUID(gFilePath);
    if (!gFileUUID) { popup("Failed to validate checksum (failed generate UUID for " + gFilePath + ")"); return; }

    QMessageBox msgBox;
    if (uuid != *gFileUUID) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The contents of " + gFilePath + " have changed.\n\nChecksums:\nold: " + uuid + "\nnew: " + *gFileUUID);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        dbUpdateModelUUID();
    }
}

void VerificationValidationWidget::dbUpdateModelUUID() {
    QString* uuid = generateUUID(*document->getFilePath());
    if (!uuid) return;
    QSqlQuery* updateQuery = new QSqlQuery(getDatabase());
    updateQuery->prepare("UPDATE Model SET uuid = ? WHERE id = ?");
    updateQuery->addBindValue(*uuid);
    updateQuery->addBindValue(modelID);
    updateQuery->exec();
    delete updateQuery;

    updateDockableHeader();
}

void VerificationValidationWidget::updateDockableHeader() {
    QSqlQuery* q = new QSqlQuery(getDatabase());
    q->prepare("SELECT uuid, filePath FROM Model WHERE id = ?");
    q->addBindValue(modelID);
    q->exec();
    if (q->next()) {
        QString uuid = q->value(0).toString();
        QString filePath = q->value(1).toString();

        QString dockableTitle = "Verification & Validation\tFile Path: "+filePath+"\tModel UUID: "+uuid;
        QLabel *title = new QLabel(dockableTitle);
        title->setObjectName("dockableHeader");
        parentDockable->setTitleBarWidget(title);
        qApp->processEvents();
    }
    delete q;
}