/*                     O B J E C T T R E E W I D G E T . H
 * BRL-CAD
 *
 * Copyright (c) 2020-2025 United States Government as represented by
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
/** @file ObjectTreeWidget.h */

#ifndef OBJECTTREEWIDGET_H
#define OBJECTTREEWIDGET_H

#include <QTreeWidget>


class Document;


class ObjectTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit ObjectTreeWidget(Document *document, QWidget *parent = nullptr);

    void build(const size_t objectId, QTreeWidgetItem* parent = nullptr);
    void destroy(const size_t objectId);
    void update();
    void select(QString selected);
    void traverseSubTree(QTreeWidgetItem *rootOfSubTree, bool traverseRoot, const std::function<bool(QTreeWidgetItem*)>& callback);
    void refreshItemTextColors();
    void setTextColor();

    // Getters
    const QHash<size_t, QTreeWidgetItem *>& getObjectIdTreeWidgetItemMap() const {
        return objectIdTreeWidgetItemMap;
    }

private:
    Document* document;

    QHash<size_t, QTreeWidgetItem*> objectIdTreeWidgetItemMap;

    QColor colorFullVisible;
    QColor colorSomeChildrenVisible;
    QColor colorInvisible;
    QColor colorDead;

signals:
    void visibilityButtonClicked(size_t objectId);
    void selectionChanged(size_t objectId);
};


#endif // OBJECTTREEWIDGET_H
