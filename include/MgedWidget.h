#ifndef MGEDWIDGET_H
#define MGEDWIDGET_H

#include <iostream>
#include <QPlainTextEdit>
#include <QTextBlock>
#include "Utils.h"
#include "QVBoxWidget.h"

#define TERMINAL_PREFIX "mged> "

class Document;
class MgedWidget : public QTextEdit {
public:
	explicit MgedWidget(Document* d, QWidget* parent = nullptr);

private:
	Document* d;
	const QString prefix;
	int baseCurPos; // any idx less than equal this should not be editable

	void clear();
	void keyPressEvent(QKeyEvent* event);
};

#endif