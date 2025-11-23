/*                        C O N S O L E . C P P
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
/** @file Console.cpp */


#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QKeyEvent>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStringListModel>
#include "Globals.h"
#include "MainWindow.h"
#include "Document.h"

#include "Console.h"


/* ---------- Console completer ---------- */


class ConsoleCompleter : public QCompleter {
public:
    ConsoleCompleter(Console* parent) {
        setParent(parent);
        popup()->setObjectName("consoleCompleter");
    }


    void updateCompletionModel(const QString& consoleText) {
        setModel(NULL);
        if (consoleText.isEmpty())
            return;

        // If last char is ' ', don't offer any completions (prior contents are presumed to be complete)
        if (consoleText.at(consoleText.length() - 1) == ' ')
            return;

        Console *console = (Console *)parent();

        // Split up and process the string's components into an argc/argv array
        QRegularExpression regExForSpaces("(\\ )");  // RegEx for ' '
        QStringList commandComponents = consoleText.split(regExForSpaces);
        QByteArrayList argvByteArray;
        std::vector<const char*> argv;
        int argc = 0;
        for (const QString& component : commandComponents) {
            if (component != "") {
                argvByteArray.append(component.toLocal8Bit());
                argv.push_back(argvByteArray.last().constData());
                ++argc;
            }
        }

        // If we only have 1 argument, it has to be a command of some sort
        if (argc == 1) {
            QStringList matches = QStringList();
            console->getParser()->CompleteCommand(argv.front(), [&matches](const char *commandMatch) -> bool {
                matches.append(QString(commandMatch));
                return true;
            });

            if (!matches.isEmpty()) {
                setCompletionMode(QCompleter::PopupCompletion);
                setModel(new QStringListModel(matches, this));
                setCaseSensitivity(Qt::CaseSensitive);
                setCompletionPrefix(commandComponents.front());
                if (popup())
                    popup()->setCurrentIndex(completionModel()->index(0, 0));
            }

            return;
        }

        // If we've got more than one argument, the last element has to be some sort of db object/path element
        // But if there is no active document, ignore it
        if (!console->getActiveDocument())
            return;

        QStringList matches = QStringList();
        console->getParser()->CompleteObject(argv.back(), [&matches](const char *objectMatch) -> bool {
            matches.append(QString(objectMatch));
            return true;
        });

        QRegularExpression regExForSlashes("(\\/)");  // RegEx for '/'
        QStringList queryForSlashes = commandComponents.back().split(regExForSlashes);
        if (queryForSlashes.size() == 1)
            console->setSplitSlash(false);
        else
            console->setSplitSlash(true);

        if (!matches.isEmpty()) {
            setCompletionMode(QCompleter::PopupCompletion);
            setModel(new QStringListModel(matches, this));
            setCaseSensitivity(Qt::CaseSensitive);
            setCompletionPrefix(queryForSlashes.back());
            if (popup())
                popup()->setCurrentIndex(completionModel()->index(0, 0));
        }
    }
};


/* ---------- Console QWidget ---------- */

Console::Console(Document *document, QWidget *parent) : activeDocument(document), QPlainTextEdit(parent), interactivePosition(documentEnd()) {
    setObjectName("console");

    setAcceptDrops(false);
    setTabChangesFocus(false);
    setUndoRedoEnabled(false);
    setMaximumBlockCount(10000);

    parser = new BRLCAD::CommandString(*getActiveDocument()->getObjectTree()->getDatabase());

    commandHistory.append("");
    commandPosition = 0;

    completer = new ConsoleCompleter(this);
    completer->setWidget(this);
    connect(completer, QOverload<const QString &>::of(&QCompleter::activated), this, &Console::insertCompletion);

    connect(this, &Console::exitRequested, Globals::mainWindow, &MainWindow::tabCloseRequested);

    prompt();
}


Console::~Console(void) {
    if (parser)
        delete parser;
}


QString Console::commandFromHistory(size_t index) {
    return commandHistory.at(index);
}


int Console::documentEnd(void) {
    QTextCursor c(document());
    c.movePosition(QTextCursor::End);
    return c.position();
}


QString& Console::commandBuffer(void) {
    return commandHistory.back();
}


void Console::updateCommandBuffer(void) {
    commandBuffer() = toPlainText().mid(interactivePosition);
}


void Console::replaceCommandBuffer(const QString& newBuffer) {
    commandBuffer() = newBuffer;

    QTextCursor c(document());
    c.setPosition(interactivePosition);
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    c.removeSelectedText();
    c.insertText(newBuffer);
}


void Console::updateCompleter(void) {
    if (parser && completer) {
        // Get the text between the current cursor position and the start of the line
        QTextCursor c = textCursor();
        c.setPosition(interactivePosition, QTextCursor::KeepAnchor);
        QString commandText = c.selectedText();

        // Call the completer to update the completion model
        completer->updateCompletionModel(commandText);

        // Place and show the completer if there are available completions
        if (completer->completionCount()) {
            // Get a QRect for the cursor at the start of the current word and then translate it down 8 pixels
            c = textCursor();
            c.movePosition(QTextCursor::StartOfWord);
            QRect cr = cursorRect(c);
            cr.translate(0, 8);
            cr.setWidth(completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width());
            completer->complete(cr);
        } else
            completer->popup()->hide();
    }
}


void Console::updateCompleterIfVisible(void) {
    if (parser && completer && completer->popup()->isVisible())
        updateCompleter();
}


void Console::selectCompletion(void) {
    if (parser && completer && completer->completionCount() == 1) {
        insertCompletion(completer->currentCompletion());
        completer->popup()->hide();
    }
}


void Console::executeCommand(void) {
    QString command = commandBuffer();
    QTextCursor c(document());
    c.movePosition(QTextCursor::End);

    // Don't store empty commands in the history, and don't execute anything
    if (command.isEmpty()) {
        c.insertText("\n");
        prompt();
        return;
    }

    // If no document is open, don't execute anything
    if (!parser) {
        c.insertText(QString("\nNo active document, can't execute!\n"));
        prompt();
        return;
    }

    // If we are currently handling a multi input command, create the correct final command
    if (handlingMultiInputs)
        command = multiInputBuffer + command;

    // Create command argv
    QRegularExpression regExForSpaces("(\\ )");  // RegEx for ' '
    QStringList commandComponents = command.split(regExForSpaces);
    QByteArrayList argvByteArray;
    std::vector<const char*> argv;
    int argc = 0;
    for (const QString& component : commandComponents) {
        if (component != "") {
            argvByteArray.append(component.toLocal8Bit());
            argv.push_back(argvByteArray.last().constData());
            ++argc;
        }
    }

    // Signal the ObjectTree that a GED command is being executed
    getActiveDocument()->getObjectTree()->cmdExecutionStarted();

    // Execute command
    BRLCAD::CommandString::State parserState = parser->Parse(argv.size(), argv.data());

    // Signal the ObjectTree that the GED command execution has ended
    getActiveDocument()->getObjectTree()->cmdExecutionEnded();

    // If command is multi input, handle it differently
    if (parserState == BRLCAD::CommandString::State::Incomplete) {
        handlingMultiInputs = true;
        multiInputBuffer = command + QString(" ");
        c.insertText(QString("\n"));
        c.insertText(QString(parser->Results()));
        parser->ClearResults();
        interactivePosition = documentEnd();
        ensureCursorVisible();
        return;
    }
    /* If we were handling a multi input command, but the last executed command is not a multi input anymore,
       it means that it is now complete, so clear all the multi input related stuff */
    if (handlingMultiInputs) {
        multiInputBuffer.clear();
        commandBuffer() = command;
        handlingMultiInputs = false;
    }

    // Update command history
    commandHistory.push_back("");
    commandPosition = commandHistory.size() - 1;

    // Print result
    QString result = parser->Results();
    // Try to keep the newlines consistent
    if (!result.isEmpty()) {
        int i;
        for (i = result.size() - 1 ; i >= 0; --i) {
            if (result[i] != '\n') {
                ++i;
                break;
            }
        }
        // If last char is not a newline, add it
        if (i == result.size())
            result.append('\n');
        // Else if there are more than one newline, keep only one
        else if (result.size() - i > 1)
            result.chop(result.size() - i - 1);
    }
    c.insertText(QString("\n"));
    c.insertText(result);
    switch (parserState) {
        case BRLCAD::CommandString::State::Success:
            // Handle "clear" command differently
            if (argvByteArray.first() == "clear")
                document()->clear();
            break;

        case BRLCAD::CommandString::State::ExitRequested:
            emit exitRequested(tabToCloseId);
            break;

        case BRLCAD::CommandString::State::NoDatabase:
            c.insertText(QString("PARSER ERROR! (NoDatabase)\n"));
            break;

        case BRLCAD::CommandString::State::InternalError:
            c.insertText(QString("PARSER ERROR! (InternalError)\n"));
            break;
    }

    // Prepare for next command
    parser->ClearResults();
    interactivePosition = documentEnd();
    prompt();
}


void Console::prompt(void) {
    textCursor().insertText("$ ");
    interactivePosition = documentEnd();
    ensureCursorVisible();
}


void Console::resizeEvent(QResizeEvent *e) {
    setMaximumBlockCount(2 * height());
    QPlainTextEdit::resizeEvent(e);
}


void Console::insertFromMimeData(const QMimeData * s) {
    QTextCursor c = textCursor();

    // Set to true if the cursor overlaps the history area
    const bool historyArea = c.anchor() < interactivePosition || c.position() < interactivePosition;

    // Avoid pasting into history
    if (historyArea)
        return;

    QPlainTextEdit::insertFromMimeData(s);

    // The text changed - make sure the command buffer knows
    updateCommandBuffer();
}


void Console::keyPressEvent(QKeyEvent* e) {
    if (completer && completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
            case Qt::Key_Tab:
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Backtab:
                e->ignore();
                return;  // let the completer do default behavior

            default:
                break;
        }
    }

    QTextCursor c = textCursor();

    // Set to true if there's a current selection
    const bool selection = c.anchor() != c.position();
    // Set to true if the cursor overlaps the history area
    const bool historyArea = c.anchor() < interactivePosition || c.position() < interactivePosition;

    // Allow copying anywhere in the console
    if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier) {
        if (selection)
            copy();
        e->accept();
        return;
    }

    // Allow cut only if the selection is limited to the interactive area
    if (e->key() == Qt::Key_X && e->modifiers() == Qt::ControlModifier) {
        if (selection && !historyArea)
            cut();
        e->accept();
        return;
    }

    // Allow paste only if the selection is in the interactive area ...
    if (e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier) {
        if (!historyArea) {
            const QMimeData* const clipboard = QApplication::clipboard()->mimeData();
            const QString text = clipboard->text();
            if (!text.isNull()) {
                c.insertText(text);
                updateCommandBuffer();
            }
        }
        e->accept();
        return;
    }

    // Force the cursor back to the interactive area
    if (historyArea && e->key() != Qt::Key_Control) {
        c.setPosition(documentEnd());
        setTextCursor(c);
    }

    switch (e->key()) {
        case Qt::Key_Up:
            e->accept();
            if (commandPosition > 0)
                replaceCommandBuffer(commandHistory[--commandPosition]);
            break;

        case Qt::Key_Down:
            e->accept();
            if (commandPosition < commandHistory.size() - 2)
                replaceCommandBuffer(commandHistory[++commandPosition]);
            else {
                commandPosition = commandHistory.size() - 1;
                replaceCommandBuffer("");
            }
            break;
        
        case Qt::Key_Left:
            if (c.position() > interactivePosition)
                QPlainTextEdit::keyPressEvent(e);
            else
                e->accept();
            break;

        case Qt::Key_Delete:
            e->accept();
            QPlainTextEdit::keyPressEvent(e);
            updateCommandBuffer();
            break;
        
        case Qt::Key_Backspace:
            e->accept();
            if (c.position() > interactivePosition) {
                QPlainTextEdit::keyPressEvent(e);
                updateCommandBuffer();
                updateCompleterIfVisible();
            }
            break;
        
        case Qt::Key_Tab:
            e->accept();
            {
                c.setPosition(c.anchor(), QTextCursor::MoveAnchor);
                c.setPosition(c.position(), QTextCursor::KeepAnchor);
                updateCompleter();
                selectCompletion();
            }
            break;
        
        case Qt::Key_Home:
            e->accept();
            c.setPosition(interactivePosition);
            setTextCursor(c);
            break;
        
        case Qt::Key_Return:
        case Qt::Key_Enter:
            e->accept();
            c.setPosition(documentEnd());
            setTextCursor(c);
            executeCommand();
            break;

        default:
            e->accept();
            QPlainTextEdit::keyPressEvent(e);
            updateCommandBuffer();
            updateCompleterIfVisible();
            break;
    }
}


void Console::focusOutEvent(QFocusEvent *e) {
    QPlainTextEdit::focusOutEvent(e);

    // For some reason the QCompleter tries to set the focus policy to NoFocus, so set it to the default WheelFocus.
    setFocusPolicy(Qt::WheelFocus);
}


void Console::insertCompletion(const QString& completion) {
    QTextCursor c = textCursor();
    c.setPosition(c.position(), QTextCursor::MoveAnchor);
    QString text = c.selectedText();
    while (c.position() > 0 && (!text.length() || (text.at(0) != ' ' && (!splitSlash || text.at(0) != '/')))) {
        c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        text = c.selectedText();
    }
    if (c.position() == interactivePosition)
        c.setPosition(c.position(), QTextCursor::MoveAnchor);
    else
        c.setPosition(c.position() + 1, QTextCursor::MoveAnchor);
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    c.insertText(completion);
    setTextCursor(c);

    updateCommandBuffer();
}
