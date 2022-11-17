#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QtWidgets/QMdiArea>
#include <unordered_map>
#include "Document.h"
#include "Dockable.h"
#include "QSSPreprocessor.h"
#include <QStatusBar>
#include <QMenuBar>
#include <QComboBox>
#include <QObject>

class Document;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStatusBar *getStatusBar() const {
        return statusBar;
    }

    QTabWidget *getDocumentArea() const {
        return documentArea;
    }

    const std::unordered_map<int, Document*> *getDocuments() const {
        return &documents;
    }

    int getActiveDocumentId() const {
        return activeDocumentId;
    }

    Dockable *getVerificationValidationDockable() const {
        return objectVerificationValidationDockable;
    }

    const int statusBarShortMessageDuration = 7000;

private:
	// UI components
    Dockable *objectTreeWidgetDockable;
    Dockable *objectPropertiesDockable;
    Dockable *objectVerificationValidationDockable;
    Dockable *toolboxDockable;
    QStatusBar *statusBar;
    QMenuBar* menuTitleBar;
    QTabWidget *documentArea;
    QPushButton * maximizeButton;
    QLabel *statusBarPathLabel;
    QComboBox * currentViewport;
    QAction* singleViewAct[4];
    QAction* verifyValidateViewportAct;
	
    // Stores pointers to all the currently opened documents. Item removed when document is closed. Key is documents ID.
    std::unordered_map<int, Document*> documents;

    // Total number of documents ever opened in application's life time. This is not decreased when closing documents.
    // A document's ID is set to documentsCount at the moment of opening it.
    int documentsCount = 0;

    // The ID of the active document.
    int activeDocumentId = -1;
	
    void prepareUi();
    void loadTheme();
    void prepareDockables();

    void newFile(); // empty new file
    void openFile(const QString& filePath);
    void openATRFile(const QString& atrFilePath);
    bool saveFile(const QString& filePath);
    bool maybeSave(int documentId, bool *cancel = nullptr);

    QAction *themeAct[2];
	
protected:
    void changeEvent(QEvent *e) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent* event) override;
	
public slots:
    void openFileDialog();
    bool saveFileId(const QString& filePath, int documentId);
    void saveAsFileDialog();
    bool saveAsFileDialogId(int documentId);
    void saveFileDefaultPath();
    bool saveFileDefaultPathId(int documentId);
    void onActiveDocumentChanged(int newIndex);
    void tabCloseRequested(int i) ;
    void objectTreeWidgetSelectionChanged(int objectId);
    void closeButtonPressed();
    void minimizeButtonPressed();
    void maximizeButtonPressed();

    void setStatusBarMessage(bool testRan, int currTest, int totalTests, int currObject, int totalObjects) {
        QString status;
        status = QString((testRan) ? "Finished" : "Started") + " running %1 / %2 tests for %3 / %4 objects";
        statusBar->showMessage(status.arg(currTest).arg(totalTests).arg(currObject).arg(totalObjects), statusBarShortMessageDuration);
        qApp->processEvents();
    }
    void setStatusBarMessage(QString msg) {
        statusBar->showMessage(msg, statusBarShortMessageDuration);
        qApp->processEvents();
    }

    void updateVerifyValidateAct(VerificationValidationWidget* vvWidget) {
        if (!vvWidget || !vvWidget->isRunningTests())
            verifyValidateViewportAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/verifyValidateIcon.png", "$Color-MenuIconVerifyValidate")));
        else
            verifyValidateViewportAct->setIcon(QIcon(":/icons/verifyValidateCancelIcon.png"));
    }
};
#endif // MAINWINDOW_H
