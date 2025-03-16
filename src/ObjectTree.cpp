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

#include <brlcad/Database/Combination.h>
#include "ObjectTree.h"
#include <QStandardItemModel>
#include "brlcad/Database/MemoryDatabase.h"


void ObjectTree::ObjectTreeCallback::operator()(const BRLCAD::Object& object)
{
    objectId = ++objectTree->lastAllocatedId;
    objectTree->getChildren()[objectId] = QVector<int>();
    objectTree->objectIdParentObjectIdMap[objectId] = parentObjectId;
    childrenNames = &objectTree->getChildren()[objectId];

    objectTree->getFullPathMap()[objectId] = currentObjectPath;

    if (const BRLCAD::Combination* combination = dynamic_cast<const BRLCAD::Combination*>(&object)) {
        traverseSubTree(combination->Tree());
    } else {
        objectTree->getDrawableObjectIds().insert(objectId);
    }
}

void ObjectTree::ObjectTreeCallback::traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node) const
{
    switch (node.Operation()) {
        case BRLCAD::Combination::ConstTreeNode::Union:
        case BRLCAD::Combination::ConstTreeNode::Intersection:
        case BRLCAD::Combination::ConstTreeNode::Subtraction:
        case BRLCAD::Combination::ConstTreeNode::ExclusiveOr:
            traverseSubTree(node.LeftOperand());
            traverseSubTree(node.RightOperand());
            break;
        
        case BRLCAD::Combination::ConstTreeNode::Not:
            traverseSubTree(node.Operand());
            break;

        case BRLCAD::Combination::ConstTreeNode::Leaf:
            objectTree->getChildren()[objectId].append(objectTree->lastAllocatedId + 1);
            QString childName = QString(node.Name());
            objectTree->getNameMap()[objectTree->lastAllocatedId + 1] = childName;
            ObjectTreeCallback callback(objectTree, childName, objectId);
            objectTree->getDatabase()->Get(node.Name(), callback);
    }
}

int ObjectTree::addTopObject(QString name)
{
    QVector<int>* childrenNames = &getChildren()[0];
    int topObjectId = lastAllocatedId + 1;
    childrenNames->append(topObjectId);
    getNameMap()[topObjectId] = name;
    ObjectTreeCallback callback(this, name, 0);
    database->Get(name.toUtf8(), callback);
    buildColorMap(topObjectId);
    return topObjectId;
}

ObjectTree::ObjectTree(BRLCAD::MemoryDatabase* database) : database(database)
{
    BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();

    objectIdChildrenObjectIdsMap[0] = QVector<int>(); // objectId of root is 0
    objectIdParentObjectIdMap[0] = -1;
    nameMap[0] = "";
    colorMap[0] = {1,1,1,false };

    while (it.Good()) {
        QString childName = it.Name();
        addTopObject(childName);
        ++it;
    }
}

void ObjectTree::traverseSubTree(const int rootOfSubTreeId, bool traverseRoot, const std::function<bool(int)>& callback)
{
    if(traverseRoot) callback(rootOfSubTreeId);
    for (int objectId : objectIdChildrenObjectIdsMap[rootOfSubTreeId]) {
        if (!callback(objectId)) continue;
        if (objectIdChildrenObjectIdsMap.contains(rootOfSubTreeId)) traverseSubTree(objectId, false, callback);
    }
}

void ObjectTree::changeVisibilityState(int objectId, bool visible)
{
    if (visible) {
        objectIdVisibilityStateMap[objectId] = FullyVisible;

        // First we go up in the tree and make necessary changes
        int ancestorId = getParent()[objectId];
        while (ancestorId != -1) {
            // if all children of the ancestor are fully visible after the change ancestor's visibility should be FullyVisible
            objectIdVisibilityStateMap[ancestorId] = FullyVisible;

            // but if there is a not fully visible child it should be SomeChildrenVisible
            for (int ancestorChildId : getChildren()[ancestorId]) {
                if (objectIdVisibilityStateMap[ancestorChildId] != FullyVisible) {
                    objectIdVisibilityStateMap[ancestorId] = SomeChildrenVisible;
                }
            }
            ancestorId = getParent()[ancestorId];
        }

        // All children of objectId should be fully visible
        traverseSubTree(objectId, false,[this] (int childId) {
            objectIdVisibilityStateMap[childId] = FullyVisible;
            return true;
        });
    } else {
        objectIdVisibilityStateMap[objectId] = Invisible;

        // First we go up in the tree and make necessary changes
        int ancestorId = getParent()[objectId];
        while (ancestorId != -1) {
            // if all children of the ancestor are invisible after the change ancestor's visibility should be Invisible
            objectIdVisibilityStateMap[ancestorId] = Invisible;

            // but if there is a not fully visible child it should be SomeChildrenVisible
            for (int ancestorChildId : getChildren()[ancestorId]) {
                if (objectIdVisibilityStateMap[ancestorChildId] != Invisible) {
                    objectIdVisibilityStateMap[ancestorId] = SomeChildrenVisible;
                }
            }
            ancestorId = getParent()[ancestorId];
        }

        // All children of objectId should be invisible
        traverseSubTree(objectId, false,[this] (int childId) {
            objectIdVisibilityStateMap[childId] = Invisible;
            return true;
        });
    }
}

void ObjectTree::buildColorMap(int rootObjectId)
{
    traverseSubTree(rootObjectId,true,[&](int objectId) {
        if(objectId==0) return true;
        const QString objectName = fullPathMap[objectId];
        const QByteArray &name = objectName.toUtf8();
        BRLCAD::Object *object = database->Get(name);
        colorMap[objectId] = ColorInfo(colorMap[objectIdParentObjectIdMap[objectId]]);
        if(const BRLCAD::Combination* combination = dynamic_cast<const BRLCAD::Combination*>(object)) {
            if (combination->HasColor()) {
                colorMap[objectId].red = combination->Red();
                colorMap[objectId].green = combination->Green();
                colorMap[objectId].blue = combination->Blue();
                colorMap[objectId].hasColor = true;
            }
        }
        return true;
    });
}
