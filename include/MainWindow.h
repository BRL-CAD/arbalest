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
#include "MouseAction.h"


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

    const int statusBarShortMessageDuration = 7000;

private:
    // ---------- UI COMPONENTS ----------
    Dockable *objectTreeWidgetDockable;
    Dockable *objectPropertiesDockable;
    Dockable *consoleDockable;
    Dockable *toolboxDockable;
    QStatusBar *statusBar;
    QTabWidget *documentArea;
    QLabel *statusBarPathLabel;
    MouseAction *m_mouseAction;
    // ---------- Menu bar ----------
    QMenuBar *menuTitleBar;
    // File menu
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *quitAct;
    // Edit menu
    QAction *selectObjectAct;
    // View menu
    QAction *resetAllViewportsAct;
    QAction *autoViewAct;
    QAction *centerViewAct;
    QComboBox *currentViewport;
    QAction *singleViewAct[4];
    QAction *toggleGridAct;
    // Raytrace menu
    QAction *raytraceAct;

    // Stores pointers to all the currently opened documents. Item removed when document is closed. Key is documents ID.
    std::unordered_map<int, Document*> documents;

    // Total number of documents ever opened in application's life time. This is not decreased when closing documents.
    // A document's ID is set to documentsCount at the moment of opening it.
    int documentsCount = 0;

    // The ID of the active document.
    int activeDocumentId = -1;

    void prepareUi();
    void loadTheme();
    void setIcons();
    void prepareDockables();

    bool saveFile(const QString& filePath);
    bool maybeSave(int documentId, bool *cancel = nullptr);

    QAction *themeAct[2];
	
protected:
    void closeEvent(QCloseEvent* event) override;
	
    void moveCameraButtonAction();
    void selectObjectButtonAction();

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
    void newFile(); // empty new file
    void openFile(const QString& filePath);
    void updateMouseButtonObjectState();
};
#endif // MAINWINDOW_H
