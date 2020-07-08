#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QMdiArea>
#include <unordered_map>
#include "Document.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Ui::MainWindow *ui;

    // Stores pointers to all the currently opened documents. Item removed when document is closed. Key is documents ID.
    std::unordered_map<int, Document*> documents;

    // Total number of documents ever opened in application's life time. This is not decreased when closing documents.
    // A document's ID is set to documentsCount at the moment of opening it.
    int documentsCount = 0;

    // The ID of the active document.
    int activeDocumentId = -1;

    void openFileDialog();
    void saveFileDialog();
    void openFile(const QString& filePath);

protected:

    void setTheme();

public slots:
    void onActiveDocumentChanged(QMdiSubWindow *window);
    void closeButtonPressed();
};
#endif // MAINWINDOW_H
