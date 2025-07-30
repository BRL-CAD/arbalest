/*                          C O N S O L E . H
 * BRL-CAD
 *
 * Copyright (c) 2025 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file Console.h */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <brlcad/CommandString/CommandString.h>
#include "Document.h"


class ConsoleCompleter;


class Console : public QPlainTextEdit {
    Q_OBJECT

public:
    Console(QWidget *parent = nullptr);
    ~Console();

    // Setters

    // Set the active document (pass nullptr if no document is opened)
    void setActiveDocument(Document *currDocument);

    void setSplitSlash(bool newSplitSlash) {
        splitSlash = newSplitSlash;
    }

    // Getters
    Document *getActiveDocument(void) {
        return activeDocument;
    }
    
    BRLCAD::CommandString *getParser(void) {
        return parser;
    }

    // Return a command from history  at specified index (why is it public?)
    QString commandFromHistory(size_t index);

private:
    // Beginning of the area of interactive input, outside which changes can't be made to the text edit contents
    int interactivePosition;

    // Stores command-history, plus the current command buffer
    QStringList commandHistory;
    
    // Stores the current position in the command-history
    size_t commandPosition;

    // Split slashes
    bool splitSlash = false;

    // Currently handling a multi inputs command
    bool handlingMultiInputs = false;

    // Command buffer for multi input command
    QString multiInputBuffer = "";

    // A custom completer
    ConsoleCompleter *completer = nullptr;

    // Current active Document
    Document *activeDocument = nullptr;

    // Current parser
    BRLCAD::CommandString *parser = nullptr;

    // Returns the end of the document
    int documentEnd(void);
    
    // References the buffer where the current un-executed command is stored
    QString& commandBuffer(void);
    // Update the contents of the command buffer from the contents of the widget
    void updateCommandBuffer(void);
    // Replace the contents of the command buffer, updating the display
    void replaceCommandBuffer(const QString& nuwBuffer);

    // Updates completer, and shows completer popup if there are more than 1 completions
    void updateCompleter(void);
    // Calls updateCompleter if the completer popup is already visible
    void updateCompleterIfVisible(void);
    // If there is exactly 1 completion, insert it and hide the completer, else do nothing
    void selectCompletion(void);

    // Implements command-execution
    void executeCommand(void);

    // Prints prompt text
    void prompt(void);

	// Tries to keep the scrollbar slider from getting too small to be usable
    void resizeEvent(QResizeEvent *e);
    void insertFromMimeData(const QMimeData * s);
    void keyPressEvent(QKeyEvent* e);
    void focusOutEvent(QFocusEvent *e);

public slots:
    void insertCompletion(const QString& completion);
};


#endif // CONSOLE_H
