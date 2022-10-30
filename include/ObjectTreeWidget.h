/*                      O B J E C T S T R E E V I E W . H
 * BRL-CAD
 *
 * Copyright (c) 2018 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/** @file ObjectTreeWidget.h
 *
 */

#ifndef OBJECTTREEWIDGET_H
#define OBJECTTREEWIDGET_H

#include <QTreeView>
#include <qtreewidget.h>
#include <QtWidgets/QStyledItemDelegate>
#include "ObjectTree.h"
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <bu.h>
class Document;
class ObjectTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit ObjectTreeWidget(Document *objectTree,   QWidget *parent = nullptr);
    void refreshItemTextColors();
    const QHash<int, QTreeWidgetItem *> &getObjectIdTreeWidgetItemMap() const;
    void build(int objectId, QTreeWidgetItem* parent = nullptr);

    enum Name { PATHNAME, BASENAME };
    enum Level { TOP, ALL };
    QStringList getSelectedObjects(const Name& name, const Level& level);
    
private:
    Document* document;
    QHash <int, QTreeWidgetItem*> objectIdTreeWidgetItemMap;

    QColor colorFullVisible;
    QColor colorSomeChildrenVisible;
    QColor colorInvisible;

signals:
    void visibilityButtonClicked(int objectId);
    void selectionChanged(int objectId);


};


#endif // OBJECTTREEWIDGET_H
