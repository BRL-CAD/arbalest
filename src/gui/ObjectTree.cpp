/*                    O B J E C T S T R E E V I E W . C P P
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
/** @file ObjectTree.cpp
 *
 *  taken from RT3/QtGUI:
 *      implementation of the objects' tree visualization
 */

#include <brlcad/Combination.h>

#include "ObjectTree.h"


ObjectTree::ObjectTree
(
    BRLCAD::ConstDatabase& database,
    QWidget*               parent
) : QTreeView(parent), m_database(database) {
    m_objectTree = new QStandardItemModel;

    setModel(m_objectTree);
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &ObjectTree::Activated
    );
}


class ObjectTreeCallback : public BRLCAD::ConstDatabase::ObjectCallback {
public:
    ObjectTreeCallback(const BRLCAD::ConstDatabase& database,
                       QStandardItem*               parentItem) : m_database(database), m_parentItem(parentItem) {}

    virtual void operator()(const BRLCAD::Object &object) {
        const BRLCAD::Combination* comb = dynamic_cast<const BRLCAD::Combination*>(&object);

        if (comb != 0)
            ListTreeNode(comb->Tree());
    }

private:
    const BRLCAD::ConstDatabase& m_database;
    QStandardItem*               m_parentItem;

    void ListTreeNode(const BRLCAD::Combination::ConstTreeNode& node) {
        switch (node.Operation()) {
            case BRLCAD::Combination::ConstTreeNode::Union:
            case BRLCAD::Combination::ConstTreeNode::Intersection:
            case BRLCAD::Combination::ConstTreeNode::Subtraction:
            case BRLCAD::Combination::ConstTreeNode::ExclusiveOr:
                ListTreeNode(node.LeftOperand());
                ListTreeNode(node.RightOperand());
                break;

            case BRLCAD::Combination::ConstTreeNode::Not:
                ListTreeNode(node.Operand());
                break;

            case BRLCAD::Combination::ConstTreeNode::Leaf:
                QStandardItem*     objectItem = new QStandardItem(node.Name());
                ObjectTreeCallback callback(m_database, objectItem);

                m_database.Get(node.Name(), callback);
                m_parentItem->appendRow(objectItem);
        }
    }
};


void ObjectTree::Rebuild(void) {
    m_objectTree->clear();

    BRLCAD::ConstDatabase::TopObjectIterator it = m_database.FirstTopObject();

    while (it.Good()) {
        QStandardItem*     objectItem = new QStandardItem(it.Name());
        ObjectTreeCallback callback(m_database, objectItem);

        m_database.Get(it.Name(), callback);
        m_objectTree->appendRow(objectItem);

        ++it;
    }
}


void ObjectTree::Activated(const QItemSelection & selected, const QItemSelection & deselected) {
    QModelIndexList selectedIndexes;

    m_database.UnSelectAll();
    selectedIndexes = selectionModel()->selectedIndexes();

    for (int i = 0; i < selectedIndexes.size(); i++) {
        const QModelIndex selectedIndex = selectedIndexes.at(i);
        m_database.Select(m_objectTree->itemFromIndex(selectedIndex)->text().toUtf8().data());
    }

    auto p = selectedIndexes.at(0);
    QString fullPath = m_objectTree->itemFromIndex(p)->text();
    p = p.parent();
    while(p.isValid()){
        fullPath = m_objectTree->itemFromIndex(p)->text() + "/" + fullPath;
        p = p.parent();
    }
    emit SelectionChanged(fullPath);
}
