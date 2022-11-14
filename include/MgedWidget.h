#ifndef MGEDWIDGET_H
#define MGEDWIDGET_H

#include <iostream>
#include <QPlainTextEdit>
#include <QTextBlock>
#include "Utils.h"
#include "QVBoxWidget.h"

class Document;
class MgedWidget : public QPlainTextEdit {
public:
	explicit MgedWidget(Document* d, QWidget* parent = nullptr);

private:
	Document* d;
	int baseCurPos; // any idx less than equal this should not be editable

	void clear();

	void keyPressEvent(QKeyEvent* event);
};

#endif