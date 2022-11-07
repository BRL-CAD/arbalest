#include "VerificationValidationWidget.h"
#include <Document.h>
#include "MainWindow.h"
using Result = VerificationValidation::Result;
using DefaultTests = VerificationValidation::DefaultTests;
using Parser = VerificationValidation::Parser;

#define SHOW_ERROR_POPUP true

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
    QString* result = (dbp) ? new QString(bu_vls_addr(dbp->ged_result_str)) : nullptr;
    if (dbp) ged_close(dbp);

    return result;
}

void VerificationValidationWidget::runTests() {
    validateChecksum();
    dbUpdateModelUUID();
    dbClearResults();
    resultTable->setRowCount(0);
    minBtn_toggle = true;
    
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
    if(!totalTests) {
        popup("No tests were selected.");
        return;
    }

    resultTableChangeSize();
    dbClearResults();
    resultTable->setRowCount(0);

    QSqlQuery* q = new QSqlQuery(getDatabase());
    QStringList selectedObjects = document->getObjectTreeWidget()->getSelectedObjects(ObjectTreeWidget::Name::PATHNAME, ObjectTreeWidget::Level::ALL);
    QSet<QString> previouslyRunTests; // don't run duplicate tests (e.g.: "title" for each object)
    for (int objIdx = 0; objIdx < selectedObjects.size(); objIdx++) {
        QString object = selectedObjects[objIdx];
        for(int i = 0; i < totalTests; i++){
            emit mainWindow->setStatusBarMessage(false, i+1, totalTests, objIdx+1, selectedObjects.size());
            int testID = itemToTestMap.at(selected_tests[i]).first;
            Test currentTest = itemToTestMap.at(selected_tests[i]).second;

            // for the current test, insert any Args that aren't in TestArg
            for (int j = 0; j < currentTest.ArgList.size(); j++) {
                Arg::Type type = currentTest.ArgList[j].type;
                QString arg = currentTest.ArgList[j].argument;
                if (type == Arg::Type::ObjectName) arg = object.split("/").last();
                else if (type == Arg::Type::ObjectPath) arg = object;

                int cnt = 0;
                q->prepare("SELECT COUNT(*) FROM TestArg WHERE testID = ? AND argIdx = ? AND arg = ? AND argType = ?");
                q->addBindValue(testID);
                q->addBindValue(currentTest.ArgList[j].argIdx);
                q->addBindValue(arg);
                q->addBindValue(type);
                dbExec(q);

                if (q->next()) cnt = q->value(0).toInt();

                if (!cnt) {
                    q->prepare("INSERT INTO TestArg (testID, argIdx, arg, argType, defaultVal) VALUES (?,?,?,?,?)");
                    q->addBindValue(testID);
                    q->addBindValue(currentTest.ArgList[j].argIdx);
                    q->addBindValue(arg);
                    q->addBindValue(type);
                    q->addBindValue(currentTest.ArgList[j].defaultValue);
                    dbExec(q);
                }
            }

            // find objectArgID associated with this object
            QString objectPlaceholder = object;
            Arg::Type type = currentTest.getObjArgType();
            if (type == Arg::Type::ObjectName)
                objectPlaceholder = objectPlaceholder.split("/").last();
            else if (type == Arg::Type::ObjectNone)
                objectPlaceholder = "";
            
            q->prepare("SELECT id FROM TestArg WHERE (argType = ? OR argType = ? or argType = ?) AND arg = ? AND testID = ?");
            q->addBindValue(Arg::Type::ObjectName);
            q->addBindValue(Arg::Type::ObjectPath);
            q->addBindValue(Arg::Type::ObjectNone);
            q->addBindValue(objectPlaceholder);
            q->addBindValue(testID);
            dbExec(q);

            if (!q->next()) { continue; }

            // run tests
            QString objectArgID = q->value(0).toString();
            QString testCommand = currentTest.getCMD(objectPlaceholder);
            if (previouslyRunTests.contains(testCommand)) continue;
            previouslyRunTests.insert(testCommand);
            const QString* terminalOutput = runTest(testCommand);

            // Update db with new arg value
            if(itemToTestMap.at(selected_tests[i]).second.hasVarArgs()){
                std::vector<Arg> newArgs = itemToTestMap.at(selected_tests[i]).second.ArgList;
                for(int j = 0; j < newArgs.size(); j++){
                    if(newArgs[j].type == Arg::Type::Dynamic){
                        q->prepare("UPDATE TestArg SET defaultVal = :newVal WHERE testID = :testId AND argIdx = :argIdx");
                        q->bindValue(":newVal", newArgs[j].defaultValue);
                        q->bindValue(":testId", testID);
                        q->bindValue(":argIdx", j+1);
                        q->exec();
                    }
                }
            }

            QString executableName = selected_tests[i]->toolTip().split(' ', Qt::SkipEmptyParts).first();
            Result* result = nullptr;

            // find proper parser
            if (QString::compare(executableName, "search", Qt::CaseInsensitive) == 0)
                result = Parser::search(testCommand, terminalOutput, currentTest);
            else if (QString::compare(executableName, "lc", Qt::CaseInsensitive) == 0)
	            result = Parser::lc(testCommand, terminalOutput, *(document->getFilePath()));
            else if (QString::compare(executableName, "gqa", Qt::CaseInsensitive) == 0)
                result = Parser::gqa(testCommand, terminalOutput, currentTest);
            else if (QString::compare(executableName, "title", Qt::CaseInsensitive) == 0)
                result = Parser::title(testCommand, terminalOutput, currentTest);

            // if parser hasn't been implemented, default
            if (!result) {
                result = new Result;
                result->resultCode = Result::Code::UNPARSEABLE;
            }

            QString resultCode = QString::number(result->resultCode);

            // insert results into db
            q->prepare("INSERT INTO TestResults (modelID, testID, objectArgID, resultCode, terminalOutput) VALUES (?,?,?,?,?)");
            q->addBindValue(modelID);
            q->addBindValue(testID);
            q->addBindValue(objectArgID);
            q->addBindValue(resultCode);
            q->addBindValue((terminalOutput) ? *terminalOutput : "");
            dbExec(q);

            QString testResultID = q->lastInsertId().toString();
            // insert issues into db
            for (Result::ObjectIssue currentIssue : result->issues) {
                q->prepare("INSERT INTO ObjectIssue (objectName, issueDescription) VALUES (?,?)");
                q->addBindValue(currentIssue.objectName);
                q->addBindValue(currentIssue.issueDescription);
                dbExec(q);

                QString objectIssueID = q->lastInsertId().toString();
                q->prepare("INSERT INTO Issues (testResultID, objectIssueID) VALUES (?,?)");
                q->addBindValue(testResultID);
                q->addBindValue(objectIssueID);
                dbExec(q);
            }
            emit mainWindow->setStatusBarMessage(true, i+1, totalTests, objIdx+1, selectedObjects.size());
            showResult(testResultID);
        }
    }

    q->prepare("SELECT md5Checksum, filePath FROM Model WHERE id = ?");
    q->addBindValue(modelID);
    dbExec(q);
    if (!q->next()) {
        popup("Failed to show modelID " + modelID);
        return;
    }

    QString md5 = q->value(0).toString();
    QString filePath = q->value(1).toString();
    delete q;
    QString dockableTitle = "Verification & Validation -- File Path: "+filePath+",    MD5: "+md5+",    Model ID: "+modelID;
    QLabel *title = new QLabel(dockableTitle);
    title->setObjectName("dockableHeader");
    parentDockable->setTitleBarWidget(title);
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
        delete dbExec("CREATE TABLE Tests (id INTEGER PRIMARY KEY, testName TEXT NOT NULL, category TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestResults"))
        delete dbExec("CREATE TABLE TestResults (id INTEGER PRIMARY KEY, modelID INTEGER NOT NULL, testID INTEGER NOT NULL, objectArgID INTEGER NOT NULL, resultCode TEXT, terminalOutput TEXT)");
    if (!getDatabase().tables().contains("Issues"))
        delete dbExec("CREATE TABLE Issues (id INTEGER PRIMARY KEY, testResultID INTEGER NOT NULL, objectIssueID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("ObjectIssue"))
        delete dbExec("CREATE TABLE ObjectIssue (id INTEGER PRIMARY KEY, objectName TEXT NOT NULL, issueDescription TEXT NOT NULL)");
    if (!getDatabase().tables().contains("TestSuites"))
        delete dbExec("CREATE TABLE TestSuites (id INTEGER PRIMARY KEY, suiteName TEXT NOT NULL, UNIQUE(suiteName))");
    if (!getDatabase().tables().contains("TestsInSuite"))
        delete dbExec("CREATE TABLE TestsInSuite (id INTEGER PRIMARY KEY, testSuiteID INTEGER NOT NULL, testID INTEGER NOT NULL)");
    if (!getDatabase().tables().contains("TestArg"))
        delete dbExec("CREATE TABLE TestArg (id INTEGER PRIMARY KEY, testID INTEGER NOT NULL, argIdx INTEGER NOT NULL, arg TEXT NOT NULL, argType INTEGER NOT NULL, defaultVal TEXT)");
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
            Test* t = DefaultTests::allTests[i];
			
            for (const QString& suiteName : t->suiteNames) {
                q->prepare("INSERT OR IGNORE INTO TestSuites VALUES (NULL, ?)");
                q->addBindValue(suiteName);
                dbExec(q);
            }
            
            q->prepare("INSERT INTO Tests (testName, category) VALUES (:testName, :category)");
            q->bindValue(":testName", t->testName);
            q->bindValue(":category", t->category);
            dbExec(q);
            QString testID = q->lastInsertId().toString();

            for (int j = 0; j < DefaultTests::allTests[i]->ArgList.size(); j++) {
                Arg::Type type = DefaultTests::allTests[i]->ArgList[j].type;

                int cnt = 0;
                q->prepare("SELECT COUNT(*) FROM TestArg WHERE testID = ? AND argIdx = ? AND arg = ? AND argType = ?");
                q->addBindValue(testID);
                q->addBindValue(DefaultTests::allTests[i]->ArgList[j].argIdx);
                q->addBindValue(DefaultTests::allTests[i]->ArgList[j].argument);
                q->addBindValue(type);
                dbExec(q);

                if (q->next()) cnt = q->value(0).toInt();

                if (!cnt) {
                    q->prepare("INSERT INTO TestArg (testID, argIdx, arg, argType, defaultVal) VALUES (?,?,?,?,?)");
                    q->addBindValue(testID);
                    q->addBindValue(DefaultTests::allTests[i]->ArgList[j].argIdx);
                    q->addBindValue(DefaultTests::allTests[i]->ArgList[j].argument);
                    q->addBindValue(type);
                    q->addBindValue(DefaultTests::allTests[i]->ArgList[j].defaultValue);
                    dbExec(q);
                }
            }
            
            for (const QString& suiteName : t->suiteNames) {
                q->prepare("SELECT id FROM TestSuites WHERE suiteName = ?");
                q->addBindValue(suiteName);
                dbExec(q);
                if (!q->next()) continue;
                QString testSuiteID = q->value(0).toString();
                q->prepare("INSERT INTO TestsInSuite (testID, testSuiteID) VALUES (?, ?)");
                q->addBindValue(testID);
                q->addBindValue(testSuiteID);
                dbExec(q);
            }
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
        if(itemToTestMap.at(test).second.hasVarArgs()) {
            QDialog* userInputDialog = new QDialog();
            userInputDialog->setModal(true);
            userInputDialog->setWindowTitle("Custom Argument Value");

            QVBoxLayout* vLayout = new QVBoxLayout();
            QFormLayout* formLayout = new QFormLayout();

            QString testName = itemToTestMap.at(test).second.testName;
            vLayout->addWidget(new QLabel("Test Name: "+ testName));
            vLayout->addSpacing(5);
            vLayout->addWidget(new QLabel("Test Command: "+ itemToTestMap.at(test).second.getCMD()));
            vLayout->addSpacing(15);

            std::vector<QLineEdit*> input_vec;
            for(int i = 0; i < itemToTestMap.at(test).second.ArgList.size();  i++){
                if(itemToTestMap.at(test).second.ArgList[i].type == Arg::Type::Dynamic){
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

            connect(setBtn, &QPushButton::clicked, [this, test, input_vec, testName](){
                bool isDefault = true;
                for(int i = 0; i < itemToTestMap.at(test).second.ArgList.size();  i++){
                    if(itemToTestMap.at(test).second.ArgList[i].type == Arg::Type::Dynamic){
                        itemToTestMap.at(test).second.ArgList[i].defaultValue = input_vec[i]->text();
                        if (DefaultTests::nameToTestMap.at(testName).ArgList[i].defaultValue != input_vec[i]->text())
                            isDefault = false;
                    }
                }

                if(isDefault){
                    test->setText(testName+" (default)");
                    test->setIcon(QIcon(":/icons/edit_default.png"));
                } else {
                    test->setText(testName);
                    test->setIcon(QIcon(":/icons/edit.png"));
                }
                test->setToolTip(itemToTestMap.at(test).second.getCMD());
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
    resultTable->setColumnCount(columnLabels.size() + 2); // add hidden columns for testResultID + object
    resultTable->setHorizontalHeaderLabels(columnLabels);
    resultTable->verticalHeader()->setVisible(false);
    resultTable->horizontalHeader()->setStretchLastSection(true);
    resultTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    addWidget(resultTable);

    // Get test list from db
    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);
    query.exec("Select id, testName, category from Tests ORDER BY category ASC");

    QStringList testIdList;
    QStringList testNameList;
    QStringList categoryList;

    while(query.next()){
        testIdList << query.value(0).toString();
    	testNameList << query.value(1).toString();
        categoryList << query.value(2).toString();
    }

    // Creat test widget item
    for (int i = 0; i < testNameList.size(); i++) {
        QListWidgetItem* item = new QListWidgetItem(testNameList[i]);
        int id = testIdList[i].toInt();

        std::vector<VerificationValidation::Arg> ArgList;
        query.prepare("Select arg, defaultVal, argType FROM TestArg Where testID = :id ORDER BY argIdx");
        query.bindValue(":id", id);
        query.exec();

        bool addedObject = false;
        while(query.next()){
            Arg::Type type = (Arg::Type) query.value(2).toInt();
            if (type == Arg::Type::ObjectName || type == Arg::Type::ObjectPath) {
                if (addedObject) continue;
                ArgList.push_back(VerificationValidation::Arg((type == Arg::Type::ObjectName) ? "$OBJECT" : "/$OBJECT", query.value(1).toString(), type));
                addedObject = true;
            }
            else ArgList.push_back(VerificationValidation::Arg(query.value(0).toString(), query.value(1).toString(), type));
        }
        Test t(testNameList[i], {}, ArgList);

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setFlags(item->flags() &  ~Qt::ItemIsSelectable);
        if(t.hasVarArgs()) {
            item->setText(item->text()+" (default)");
            item->setIcon(QIcon(":/icons/edit_default.png"));
        }

        itemToTestMap.insert(make_pair(item, make_pair(id, t)));
        idToItemMap.insert(make_pair(id, item));
        item->setToolTip(itemToTestMap.at(item).second.getCMD());
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

    resultTable->setShowGrid(false);
    resultTable->setStyleSheet("QTableWidget::item {border-bottom: 0.5px solid #3C3C3C;}");
    resultTable->setColumnHidden(OBJECT_COLUMN, true);
    resultTable->setColumnHidden(TEST_RESULT_ID_COLUMN, true);
	
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
    resultTable->setColumnWidth(RESULT_CODE_COLUMN, this->width() * 0.025);
    resultTable->setColumnWidth(TEST_NAME_COLUMN, this->width() * 0.125);
    resultTable->setColumnWidth(DESCRIPTION_COLUMN, this->width() * 0.60);
    resultTable->setColumnWidth(OBJPATH_COLUMN, this->width() * 0.25);
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
    if (!q->next()) { return; }
    int testID = q->value(0).toInt();
    Test currentTest = itemToTestMap.at(idToItemMap.at(testID)).second;
    QString object = resultTable->item(row, OBJECT_COLUMN)->text();
    QString testCommand = currentTest.getCMD(object);

    QSqlQuery* q2 = new QSqlQuery(getDatabase());
    q2->prepare("SELECT TestResults.terminalOutput, TestResults.resultCode FROM TestResults INNER JOIN TestArg ON TestResults.objectArgID = TestArg.id WHERE TestResults.testID = ? AND (TestArg.argType = ? OR TestArg.argType = ? OR TestArg.argType = ?) AND TestArg.arg = ?");
    q2->addBindValue(testID);
    q2->addBindValue(Arg::Type::ObjectName);
    q2->addBindValue(Arg::Type::ObjectNone);
    q2->addBindValue(Arg::Type::ObjectPath);
    
    QString objectPlaceholder = object;
    Arg::Type type = currentTest.getObjArgType();
    if (type == Arg::Type::ObjectName)
        objectPlaceholder = objectPlaceholder.split("/").last();
    else if (type == Arg::Type::ObjectNone)
        objectPlaceholder = "";
    q2->addBindValue(objectPlaceholder);
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
    testCmdLabel->setFixedWidth(750);
    detailLayout->addWidget(testCmdLabel);
    detailLayout->addSpacing(10);
    detailLayout->addWidget(resultCodeHeader);
    detailLayout->addWidget(new QLabel(resultCode));
    detailLayout->addSpacing(10);
    detailLayout->addWidget(descriptionHeader);
    detailLayout->addWidget(new QLabel(description));
    detailLayout->addSpacing(10);
    detailLayout->addWidget(rawOutputHeader);

    QTextEdit* rawOutputBox = new QTextEdit();
    QPalette rawOutputBox_palette = rawOutputBox->palette();
    rawOutputBox_palette.setColor(QPalette::Base, Qt::black);
    rawOutputBox->setPalette(rawOutputBox_palette);
    rawOutputBox->setFontWeight(QFont::DemiBold);
    rawOutputBox->setTextColor(QColor("#39ff14"));
    rawOutputBox->append("arbalest> "+testCommand+"\n");
    rawOutputBox->setTextColor(Qt::white);
    rawOutputBox->append(terminalOutput);
    rawOutputBox->setReadOnly(true);
    rawOutputBox->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);

    detailLayout->addWidget(rawOutputBox);
    detail_dialog->setLayout(detailLayout);
    detail_dialog->exec();
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

    q->prepare("SELECT TestArg.arg FROM TestArg INNER JOIN TestResults ON TestArg.id = TestResults.objectArgID WHERE TestResults.id = ?");
    q->addBindValue(testResultID);
    dbExec(q);

    if (!q->next()) {
        popup("Failed to grab associated object for test result #" + testResultID);
        return;
    }

    QString object = q->value(0).toString();
    QString iconPath = "";
    QString objectName;
    QString issueDescription;

    QTableWidgetItem* previousRowObject = resultTable->item(resultTable->rowCount()-1, OBJECT_COLUMN);
    if (!previousRowObject || previousRowObject->text() != object) {
        resultTable->insertRow(resultTable->rowCount());
        resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem("Results for '" + ((object.isEmpty()) ? "miscellaneous" : object) + "'"));
        resultTable->item(resultTable->rowCount()-1, TEST_NAME_COLUMN)->setForeground(QBrush(QColor(Globals::theme->process("$Color-FullyVisibleObjectText"))));
        QFont f;
        f.setBold(true);
        resultTable->item(resultTable->rowCount()-1, TEST_NAME_COLUMN)->setFont(f);
    }

    if (resultCode == Result::Code::PASSED) {
        resultTable->insertRow(resultTable->rowCount());
        iconPath = ":/icons/passed.png";
        resultTable->setItem(resultTable->rowCount()-1, RESULT_CODE_COLUMN, new QTableWidgetItem(QIcon(iconPath), ""));
        resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem(testName));
        resultTable->setItem(resultTable->rowCount()-1, TEST_RESULT_ID_COLUMN, new QTableWidgetItem(testResultID));
        resultTable->setItem(resultTable->rowCount()-1, OBJECT_COLUMN, new QTableWidgetItem(object));
    } 

    else if (resultCode == Result::Code::UNPARSEABLE) {
        resultTable->insertRow(resultTable->rowCount());
        iconPath = ":/icons/unparseable.png";
        resultTable->setItem(resultTable->rowCount()-1, RESULT_CODE_COLUMN, new QTableWidgetItem(QIcon(iconPath), ""));
        resultTable->setItem(resultTable->rowCount()-1, TEST_NAME_COLUMN, new QTableWidgetItem(testName));
        resultTable->setItem(resultTable->rowCount()-1, TEST_RESULT_ID_COLUMN, new QTableWidgetItem(testResultID));
        resultTable->setItem(resultTable->rowCount()-1, OBJECT_COLUMN, new QTableWidgetItem(object));
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
            resultTable->setItem(resultTable->rowCount()-1, TEST_RESULT_ID_COLUMN, new QTableWidgetItem(testResultID));
            resultTable->setItem(resultTable->rowCount()-1, OBJECT_COLUMN, new QTableWidgetItem(object));
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

void VerificationValidationWidget::resultTableChangeSize() {
    if(minBtn_toggle){
        minBtn->setIcon(QIcon(":/icons/collapse.png"));
        parentDockable->widget()->setVisible(true);
        minBtn_toggle = false;
    } else {
        minBtn->setIcon(QIcon(":/icons/expand.png"));
        parentDockable->widget()->setVisible(false);
        minBtn_toggle = true;
    }
}

void VerificationValidationWidget::updateDockableHeader() {
    QSqlQuery* q = new QSqlQuery(getDatabase());
    q->prepare("SELECT uuid, filePath FROM Model WHERE id = ?");
    q->addBindValue(modelID);
    q->exec();
    if (q->next()) {
        QString uuid = q->value(0).toString();
        QString filePath = q->value(1).toString();

        QString dockableTitle = "Verification & Validation\tFile Path: "+filePath+" \tModel UUID: "+uuid;
        QLabel *title = new QLabel(dockableTitle);
        minBtn = new QToolButton();
        minBtn->setIcon(QIcon(":/icons/expand.png"));
        minBtn_toggle = true;
        QHBoxLayout* h_layout = new QHBoxLayout();
        h_layout->addWidget(title);
        h_layout->addWidget(minBtn);
        QWidget* titleWidget = new QWidget();
        titleWidget->setLayout(h_layout);
        title->setObjectName("dockableHeader");
        parentDockable->setTitleBarWidget(titleWidget);
        parentDockable->widget()->setVisible(false);
        qApp->processEvents();
    }
    delete q;
    // Result min button
    connect(minBtn, SIGNAL(clicked()), this, SLOT(resultTableChangeSize()));
}