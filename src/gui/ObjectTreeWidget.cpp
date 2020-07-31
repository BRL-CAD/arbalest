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
/** @file ObjectTreeWidget.cpp
 *
 */

#include <brlcad/Combination.h>
#include "ObjectTreeWidget.h"



ObjectTreeWidget::ObjectTreeWidget (ObjectTree *objectTree, QWidget *parent) : QTreeView(parent), objectTree(objectTree) {
    treeModel = buildRoot();
    setModel(treeModel);
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(selectionModel(),&QItemSelectionModel::selectionChanged, this, &ObjectTreeWidget::selectionChangedInternal);
}


class ObjectTreeWidgetCallback : public BRLCAD::ConstDatabase::ObjectCallback {
public:
    ObjectTreeWidgetCallback(BRLCAD::ConstDatabase& database,
                       QStandardItem*               parentItem) : database(database), parentItem(parentItem) {}

    void operator()(const BRLCAD::Object &object) override
    {
        const BRLCAD::Combination* comb = dynamic_cast<const BRLCAD::Combination*>(&object);

        if (comb != nullptr) {
            iterateBinaryTree(comb->Tree());
        }
    }

private:
    BRLCAD::ConstDatabase& database;
    QStandardItem*  parentItem;

    void iterateBinaryTree(const BRLCAD::Combination::ConstTreeNode& node) const
    {
        switch (node.Operation()) {
            case BRLCAD::Combination::ConstTreeNode::Union:
            case BRLCAD::Combination::ConstTreeNode::Intersection:
            case BRLCAD::Combination::ConstTreeNode::Subtraction:
            case BRLCAD::Combination::ConstTreeNode::ExclusiveOr:
                iterateBinaryTree(node.LeftOperand());
                iterateBinaryTree(node.RightOperand());
                break;

            case BRLCAD::Combination::ConstTreeNode::Not:
                iterateBinaryTree(node.Operand());
                break;

            case BRLCAD::Combination::ConstTreeNode::Leaf:
                QStandardItem*     objectItem = new QStandardItem(node.Name());
                ObjectTreeWidgetCallback callback(database, objectItem);

                database.Get(node.Name(), callback);
                parentItem->appendRow(objectItem);
        }
    }
};



QStandardItem* ObjectTreeWidget::build(const int objectId) const
{
    QStandardItem* objectItem = new QStandardItem(objectTree->getNameMap()[objectId]);
    for (int childObjectId : objectTree->getTree()[objectId])
    {
        objectItem->appendRow(build(childObjectId));
    }
    return objectItem;
}

QStandardItemModel* ObjectTreeWidget::buildRoot() const
{
	const int objectId = objectTree->getRootObjectId();
    QStandardItemModel* objectItem = new QStandardItemModel();
    for (int childObjectId : objectTree->getTree()[objectId])
    {
        objectItem->appendRow(build(childObjectId));
    }
    return objectItem;
}


void ObjectTreeWidget::selectionChangedInternal(const QItemSelection & selected, const QItemSelection & deselected) {
	QModelIndex selectedIndex = selected.indexes().at(0);
    QString fullPath;
	
    while(selectedIndex.isValid()){
        fullPath = "/" + treeModel->itemFromIndex(selectedIndex)->text() + fullPath;
        selectedIndex = selectedIndex.parent();
    }
	
    emit selectionChanged(fullPath);
}
