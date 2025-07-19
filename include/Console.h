#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>


class Console : public QPlainTextEdit {
    Q_OBJECT

public:
    Console(QWidget *parent = nullptr);
};


#endif // CONSOLE_H
