//
// Created by isaacy13 on 11/13/2022.
//
#include "MgedWidget.h"
#include "Document.h"

// TODO: test undo/redo, select all -> delete, etc.
// TODO: fix shortcuts (e.g.: fn+left)
// TODO: implement CTRL+C
// TODO: consider case where baseCurPos overflows

MgedWidget::MgedWidget(Document* d, QWidget* parent) : QPlainTextEdit(TERMINAL_PREFIX), d(d), prefix(TERMINAL_PREFIX), baseCurPos(prefix.size()) {}

void MgedWidget::keyPressEvent(QKeyEvent* event) {
	QTextCursor cursor = textCursor();
	int key = event->key();
	if (key != Qt::Key_Up && key != Qt::Key_Down && key != Qt::Key_Left && key != Qt::Key_Right) {
		if (cursor.position() == baseCurPos && key == Qt::Key_Backspace) return;
		else if (cursor.position() < baseCurPos) {
			cursor.setPosition(baseCurPos);
			setTextCursor(cursor);
			return;
		}
	}

	QPlainTextEdit::keyPressEvent(event);

	if (key == Qt::Key_Return) {
		QTextDocument* doc = document();
		QTextBlock tb = doc->findBlockByLineNumber(doc->lineCount() - 2);
		QString cmd = tb.text();
		cmd = cmd.right(cmd.size()-prefix.size());
		if (cmd == "clear") {
			clear();
			return;
		}

		QString result = mgedRun(cmd, *(d->getFilePath()));
		insertPlainText(result);

		insertPlainText("\n");
		insertPlainText(prefix);
		baseCurPos = textCursor().position();
	}
}

void MgedWidget::clear() {
	QPlainTextEdit::clear();
	setPlainText(prefix);
	baseCurPos = prefix.size();
	QTextCursor cursor = textCursor();
	cursor.setPosition(baseCurPos);
	setTextCursor(cursor);
}