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
  */

#include <brlcad/Combination.h>
#include "ObjectTree.h"
#include <QStandardItemModel>
#include "MemoryDatabase.h"


class ObjectTreeCallback : public BRLCAD::ConstDatabase::ObjectCallback {
	
public:
    ObjectTreeCallback(ObjectTree *objectTree, QString &objectName) :objectTree(objectTree), objectName(objectName) {}

    void operator()(const BRLCAD::Object& object) override
    {
        id = ++objectTree->lastAllocatedId;
        objectTree->getTree()[id] = QVector<int>();
        childrenNames = &objectTree->getTree()[id];
    	
        const BRLCAD::Combination* comb = dynamic_cast<const BRLCAD::Combination*>(&object);
    	
        if (comb != nullptr) {
            ListTreeNode(comb->Tree());
        }
    }

private:
    int id=-1;
    ObjectTree* objectTree = nullptr;
    QString objectName;
    QVector<int>* childrenNames = nullptr;
	
    void ListTreeNode(const BRLCAD::Combination::ConstTreeNode& node) const
    {
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
            objectTree->getTree()[id].append(objectTree->lastAllocatedId + 1);
            QString childName = QString(node.Name());
            objectTree->getNameMap()[objectTree->lastAllocatedId + 1] = childName;
            ObjectTreeCallback callback(objectTree, childName);
            objectTree->getDatabase()->Get(node.Name(), callback);
        }
    }
};


ObjectTree::ObjectTree (BRLCAD::MemoryDatabase* database) :  database(database) {
    
    BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();

    getTree()[0] = QVector<int>();
    QVector<int>* childrenNames = &getTree()[0];

    while (it.Good()) {
        QString childName = it.Name();
        childrenNames->append(lastAllocatedId+1);
        getNameMap()[lastAllocatedId + 1] = childName;
        ObjectTreeCallback callback(this, childName);
        database->Get(it.Name(), callback);
        ++it;
    }
}

