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


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
	
private:
	// UI components
    Dockable *objectTreeWidgetDockable;
    Dockable *objectPropertiesDockable;
    Dockable *toolboxDockable;
    QStatusBar *statusBar;
    QMenuBar* menuTitleBar;
    QTabWidget *documentArea;
    QPushButton * maximizeButton;
    QLabel *statusBarPathLabel;
	
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

    void newFile();
    void openFile(const QString& filePath);
    bool saveFile(const QString& filePath);

    const int statusBarShortMessageDuration = 7000;

	
protected:
    void changeEvent(QEvent *e) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

	
public slots:
    void openFileDialog();
    void saveAsFileDialog();
    void saveFileDefaultPath();
    void onActiveDocumentChanged(int newIndex);
    void tabCloseRequested(int i) const;
    void objectTreeWidgetSelectionChanged(int objectId);
    void closeButtonPressed();
    void minimizeButtonPressed();
    void maximizeButtonPressed();

};
#endif // MAINWINDOW_H
