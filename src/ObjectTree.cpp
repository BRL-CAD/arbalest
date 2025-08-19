/*                        O B J E C T T R E E . C P P
 * BRL-CAD
 *
 * Copyright (c) 2018-2025 United States Government as represented by
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
 /** @file ObjectTree.cpp */


#include <brlcad/Database/Combination.h>
#include "ObjectTree.h"
#include <QStandardItemModel>
#include "brlcad/Database/MemoryDatabase.h"
#include <QString>
#include <brlcad/CommandString/CommandString.h>


// ---------- OBJECT TREE ITEM DATA ----------

void ObjectTreeItemData::addOp(BRLCAD::Combination::ConstTreeNode::Operator op) {
    getChildrenOps().append(op);
}


// ---------- OBJECT TREE ITEM ----------

ObjectTreeItem::ObjectTreeItem(ObjectTreeItemData* data, size_t uniqueObjectId) : data(data), uniqueObjectId(uniqueObjectId) {
    // When I create an item, I also need to give the item to the item data that it references 
    getItemsWithSameData().append(this);
}

void ObjectTreeItem::addChild(ObjectTreeItem *itemParent) {
    getChildren().append(itemParent);
}

bool ObjectTreeItem::isRoot(void) {
    if (getParent() == nullptr)
        return true;
    return false;
}

QString ObjectTreeItem::getPath(void) {
    if (isRoot())
        return QString("");
    return getParent()->getPath() + "/" + getName();

}

// ---------- OBJECT TREE CALLBACK ----------

/* Warning: ObjectTreeCallback() (used to construct the object tree of a database) assumes that in a database
   all occurrences of a certain object (with a unique name) must have all the same children and operations.
   This means that, if there was a situation where the same combination appears more times in the database with
   different children/operations, for example:
        "combination.c"                            "combination.c"
               |__________ u "sph1.s"                     |__________ u "sph3.s"
               |__________ - "sph2.s"                     |__________ u "sph2.s"
   then the tree would be constructed so that the "combination.c" item data will have the children and operations
   equal to that of the first occurrence that it meets while creating the tree */
void ObjectTree::ObjectTreeCallback::operator()(const BRLCAD::Object& object) {
    currItem = objectTree->getItems()[objectTree->lastAllocatedId];

    // If the object is a combination, iter through its children, else it means that it is drawable
    if (const BRLCAD::Combination* combination = dynamic_cast<const BRLCAD::Combination*>(&object)) {
        if (combination->HasColor()) {
            currItem->getColorInfo().red = combination->Red();
            currItem->getColorInfo().green = combination->Green();
            currItem->getColorInfo().blue = combination->Blue();
            currItem->getColorInfo().hasColor = true;
        }
        traverseSubTree(combination->Tree());
    } else
        currItem->getData()->setIsDrawableFlag(true);

    /* The fact that I am in this function in the first place means that the item data exists.
       If a combination refers to objects that are not present in the database, then the callback
       function would not be called by Database::Get().
       The isAlive property is set to true AFTER iterating through all children of the current item data
       (if it is a combination), so that, if during the tree construction I meet this same item data, I
       will know that it is already fully constructed and that his children are already assigned */
    currItem->getData()->setIsAliveFlag(true);
}


void ObjectTree::ObjectTreeCallback::traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node) {
	switch (node.Operation()) {
        // If Union/Intersection/Subtraction/ExclusiveOr, access children
        case BRLCAD::Combination::ConstTreeNode::Union:
        case BRLCAD::Combination::ConstTreeNode::Intersection:
        case BRLCAD::Combination::ConstTreeNode::Subtraction:
        case BRLCAD::Combination::ConstTreeNode::ExclusiveOr:
            currOp = node.Operation();
            traverseSubTree(node.LeftOperand());
            currOp = node.Operation();
            traverseSubTree(node.RightOperand());
            break;

        // If Not, access child
        case BRLCAD::Combination::ConstTreeNode::Not:
            currOp = node.Operation();
            traverseSubTree(node.Operand());
            break;
        
        // If Leaf, then create a new item
        case BRLCAD::Combination::ConstTreeNode::Leaf:
            ObjectTreeItem *newItem = objectTree->addNewObjectTreeItem(QString(node.Name()));
            newItem->setParent(currItem);
            currItem->addChild(newItem);
            // If the current item data is "not alive", it means the it is not fully created yet, so addOp
            if (!currItem->isAlive())
                currItem->getData()->addOp(currOp);
            // Get new object and loop through his children with callback
            ObjectTreeCallback callback(objectTree);
            objectTree->getDatabase()->Get(node.Name(), callback);
	}
}


// ---------- OBJECT TREE ----------

ObjectTree::ObjectTree(BRLCAD::MemoryDatabase* database) : database(database) {
    // Create root item and root item data (parent = nullptr, empty name, objectId = 0)
    QString rootName = "";
    ObjectTreeItemData *rootItemData = new ObjectTreeItemData(rootName);
    getItemsData().insert(rootName, rootItemData);
    ObjectTreeItem *rootItem = new ObjectTreeItem(rootItemData, 0);
    getItems().insert(0, rootItem);
    rootItem->getData()->setIsAliveFlag(true);
    rootItem->setParent(nullptr);

    // Loop through all top level objects
	BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();
    while (it.Good()) {
        addTopObject(QString(it.Name()));
        ++it;
    }
}

ObjectTreeItem *ObjectTree::addNewObjectTreeItem(QString name) {
    // If the item name is new, it means that the item data is new, so create it. Else grab the existing one
    ObjectTreeItemData *newItemData;
    QHash<QString, ObjectTreeItemData*>::const_iterator it = getItemsData().find(name);
    if (it == getItemsData().end()) {
        newItemData = new ObjectTreeItemData(name);
        getItemsData().insert(name, newItemData);
    } else
        newItemData = it.value();
    
    // Then create the actual item with the grabbed item data
    ObjectTreeItem *newItem = new ObjectTreeItem(newItemData, ++lastAllocatedId);
    getItems().insert(lastAllocatedId, newItem);
    return newItem;
}


void ObjectTree::traverseSubTree(ObjectTreeItem *rootOfSubTree, bool traverseRoot, const std::function<bool(ObjectTreeItem*)>& callback) {
	if (traverseRoot) callback(rootOfSubTree);
	for (ObjectTreeItem *item : rootOfSubTree->getChildren()) {
		if (!callback(item)) continue;
		if (!item->getChildren().empty()) traverseSubTree(item, false, callback);
	}
}


void ObjectTree::changeVisibilityState(size_t objectId, bool visible) {
    ObjectTreeItem *item = getItems()[objectId];
    if (visible) {
        item->setVisibilityState(ObjectTreeItem::FullyVisible);

        // First we go up in the tree and make the necessary changes
        ObjectTreeItem *parentItem = item->getParent();
        while (!parentItem->isRoot()) {
            // If all children of the ancestor are FullyVisible after the change ancestor's visibility should be FullyVisible
            parentItem->setVisibilityState(ObjectTreeItem::FullyVisible);

            // But if there is a not FullyVisible child it should be SomeChildrenVisible
            for (ObjectTreeItem *parentItemChild : parentItem->getChildren()){
                if (parentItemChild->getVisibilityState() != ObjectTreeItem::FullyVisible) {
                    parentItem->setVisibilityState(ObjectTreeItem::SomeChildrenVisible);
                    break;
                }
            }
            parentItem = parentItem->getParent();
        }

        // Then we go down in the tree and make all children FullyVisible
        traverseSubTree(item, false, [](ObjectTreeItem *item) {
            item->setVisibilityState(ObjectTreeItem::FullyVisible);
            return true;
        });
    } else {
        item->setVisibilityState(ObjectTreeItem::Invisible);

        // First we go up in the tree and make the necessary changes
        ObjectTreeItem *parentItem = item->getParent();
        while (!parentItem->isRoot()) {
            // If all children of the ancestor are Invisible after the change ancestor's visibility should be Invisible
            parentItem->setVisibilityState(ObjectTreeItem::Invisible);

            // But if there is a not Invisible child it should be SomeChildrenVisible
            for (ObjectTreeItem *parentItemChild : parentItem->getChildren()){
                if (parentItemChild->getVisibilityState() != ObjectTreeItem::Invisible) {
                    parentItem->setVisibilityState(ObjectTreeItem::SomeChildrenVisible);
                    break;
                }
            }
            parentItem = parentItem->getParent();
        }

        // Then we go down in the tree and make all children Invisible
        traverseSubTree(item, false, [](ObjectTreeItem *item) {
            item->setVisibilityState(ObjectTreeItem::Invisible);
            return true;
        });
    }
}


size_t ObjectTree::addTopObject(QString name) {
    ObjectTreeItem *topLevelItem = addNewObjectTreeItem(name);
    ObjectTreeItem *rootItem = getItems()[0];
    topLevelItem->setParent(rootItem);
    rootItem->addChild(topLevelItem);
    // Get top level object and loop through his children with callback
    ObjectTreeCallback callback(this);
    database->Get(name.toUtf8(), callback);
	return lastAllocatedId;
}


// This method comes from the old ObjectTree (before PR#66)
/*void ObjectTree::buildColorMap(int rootObjectId) {
	traverseSubTree(rootObjectId,true,[&](size_t objectId){
		if (objectId == 0)return true;
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
}*/
