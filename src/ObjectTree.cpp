/*                        O B J E C T T R E E . C P P
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
 /** @file ObjectTree.cpp */

#include "Globals.h"
#include "MainWindow.h"

#include "ObjectTree.h"


// ---------- OBJECT TREE ITEM DATA ----------

void ObjectTreeItemData::addOp(BRLCAD::Combination::ConstTreeNode::Operator op) {
    getChildrenOps().append(op);
}


// ---------- OBJECT TREE ITEM ----------

ObjectTreeItem::ObjectTreeItem(ObjectTreeItemData* data, size_t uniqueObjectId) : data(data), uniqueObjectId(uniqueObjectId) {
    // When I create an item, I also need to give the item to the item data that it references
    getItemsWithSameData().append(this);
}

ObjectTreeItem::~ObjectTreeItem() {
    // When I destroy an item, I also need to delete the item from the item data that it references
    getItemsWithSameData().remove(getItemsWithSameData().indexOf(this));
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


// ---------- DATABASE CHANGE HANDLER ----------

// Function given to BRLCAD::ConstDatabase::RegisterChangeSignalHandler
void ObjectTree::databaseChangeHandler(const char* objectName, BRLCAD::ConstDatabase::ChangeType changeType) {
    if (objectName == nullptr)
        return;

    // If no document is open, return
    Document* currDocument = Globals::mainWindow->getActiveDocument();
    if (currDocument == nullptr)
        return;

    // If a GED command is not being executed, return
    ObjectTree* currObjectTree = currDocument->getObjectTree();
    if (!currObjectTree->isCmdBeingExecuted())
        return;

    QString name = objectName;
    switch (changeType) {
        case BRLCAD::ConstDatabase::ChangeType::Addition:
            currObjectTree->queueAddObjectHandler(name);
            break;

        case BRLCAD::ConstDatabase::ChangeType::Modification:
            currObjectTree->queueModifyObjectHandler(name);
            break;

        case BRLCAD::ConstDatabase::ChangeType::Removal:
            currObjectTree->queueRemoveObjectHandler(name);
            break;

        default:
            break;
    }
}


// ---------- BUILD OBJECT TREE CALLBACK ----------

/* Warning: BuildObjectTreeClbk() (used to construct the object tree of a database) assumes that in a database
   all occurrences of a certain object (with a unique name) must have all the same children and operations.
   This means that, if there was a situation where the same combination appears more times in the database with
   different children/operations, for example:
        "combination.c"                            "combination.c"
               |__________ u "sph1.s"                     |__________ u "sph3.s"
               |__________ - "sph2.s"                     |__________ u "sph2.s"
   then the tree would be constructed so that the "combination.c" item data will have the children and operations
   equal to that of the first occurrence that it meets while creating the tree */
void ObjectTree::BuildObjectTreeClbk::operator()(const BRLCAD::Object& object) {
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


void ObjectTree::BuildObjectTreeClbk::traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node) {
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
            BuildObjectTreeClbk callback(objectTree);
            objectTree->getDatabase()->Get(node.Name(), callback);
	}
}


// ---------- UPDATE OBJECT TREE CALLBACK ----------

// Used to update the object tree of a database
void ObjectTree::UpdateObjectTreeClbk::operator()(const BRLCAD::Object& object) {
    // If the object is a combination, iter through its children, and build tempBuffer and tempOpBuffer
    if (const BRLCAD::Combination* combination = dynamic_cast<const BRLCAD::Combination*>(&object))
        traverseSubTree(combination->Tree());

	qsizetype childrenBufferPos;
	qsizetype childrenOpBufferPos;

    QVector<ObjectTreeItem *>& itemsWithThisData = currItemData->getItemsWithThisData();
    if (!itemsWithThisData.isEmpty()) {
        /* All itemsWithThisData have the same children (different items, but with same item data),
           so I can loop through the children of the first itemWithThisData, and using its children
           position in the QVector (var "i") to modify the children of ALL itemsWithThisData.
           This means that we are basically iterating over the children of the ObjectTreeItemData */
        ObjectTreeItem* firstItem = itemsWithThisData[0];
        QVector<ObjectTreeItem *>& childrenOfFirstItem = firstItem->getChildren();
        for (qsizetype i = 0; i != childrenOfFirstItem.size(); ++i) {
            childrenBufferPos = childrenBuffer.indexOf(childrenOfFirstItem[i]->getName());
            // If the child already is a child of the item data, remove it from the childrenBuffer
            if (childrenBufferPos != -1)
                childrenBuffer.remove(childrenBufferPos);
            // Else, it means that the child should not be a child of the item data, so remove it
            else {
                for (ObjectTreeItem* sharedItem : currItemData->getItemsWithThisData()) {
                    ObjectTreeItem* child = sharedItem->getChildren()[i];
                    sharedItem->getChildren().remove(i);
                    objectTree->deleteObjectTreeItem(child);
                }
                --i;
            }
        }
    }
    // All the names that are that are still in childrenBuffer are new children of item data, so add them
    for (childrenBufferPos = 0; childrenBufferPos < childrenBuffer.size(); ++childrenBufferPos) {
        for (ObjectTreeItem* sharedItem : currItemData->getItemsWithThisData()) {
            ObjectTreeItem *newItem = objectTree->addNewObjectTreeItem(childrenBuffer.at(childrenBufferPos));
            newItem->setParent(sharedItem);
            sharedItem->addChild(newItem);
        }
    }
}


void ObjectTree::UpdateObjectTreeClbk::traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node) {
	switch (node.Operation()) {
        // If Union/Intersection/Subtraction/ExclusiveOr, access children
        case BRLCAD::Combination::ConstTreeNode::Union:
        case BRLCAD::Combination::ConstTreeNode::Intersection:
        case BRLCAD::Combination::ConstTreeNode::Subtraction:
        case BRLCAD::Combination::ConstTreeNode::ExclusiveOr:
            childrenOpBuffer.append(node.Operation());
            traverseSubTree(node.LeftOperand());
            childrenOpBuffer.append(node.Operation());
            traverseSubTree(node.RightOperand());
            break;

        // If Not, access child
        case BRLCAD::Combination::ConstTreeNode::Not:
            childrenOpBuffer.append(node.Operation());
            traverseSubTree(node.Operand());
            break;

        // If Leaf, then create a new item
        case BRLCAD::Combination::ConstTreeNode::Leaf:
            childrenBuffer.append(QString(node.Name()));
	}
}


// ---------- OBJECT TREE ----------

ObjectTree::ObjectTree(BRLCAD::MemoryDatabase* database, Document* document) : database(database), document(document) {
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

    database->RegisterChangeSignalHandler(databaseChangeHandlerVar);

    connect(this, &ObjectTree::addObjectHandlerSignal, this, &ObjectTree::addObjectHandler, Qt::QueuedConnection);
    connect(this, &ObjectTree::modifyObjectHandlerSignal, this, &ObjectTree::modifyObjectHandler, Qt::QueuedConnection);
    connect(this, &ObjectTree::removeObjectHandlerSignal, this, &ObjectTree::removeObjectHandler, Qt::QueuedConnection);
}


ObjectTree::~ObjectTree() {
    database->DeRegisterChangeSignalHandler(databaseChangeHandlerVar);
}


void ObjectTree::updateObjectTree() {
    QVector<QString> childrenBuffer = {};
	qsizetype childrenBufferPos;

    // Loop through all top level objects (which are the children of root)
	BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();
    while (it.Good()) {
        childrenBuffer.append(QString(it.Name()));
        ++it;
    }
    ObjectTreeItem *rootItem = getRootItem();
    QVector<ObjectTreeItem *>& childrenOfRoot = rootItem->getChildren();
    qsizetype i = 0;
    while (i != childrenOfRoot.size()) {
        ObjectTreeItem* child = childrenOfRoot[i];
        childrenBufferPos = childrenBuffer.indexOf(child->getName());
        if (childrenBufferPos != -1) {
            // If the child already is a child of root, remove it from the childrenBuffer
            childrenBuffer.remove(childrenBufferPos);
            ++i;
        } else {
            // Else, it means that the child should not be a child of root, so remove it
            childrenOfRoot.remove(i);
            deleteObjectTreeItem(child);
        }
    }
    // All the names that are that are still in childrenBuffer are new children of root, so add them
    for (childrenBufferPos = 0; childrenBufferPos < childrenBuffer.size(); ++childrenBufferPos) {
        ObjectTreeItem *newItem = addNewObjectTreeItem(childrenBuffer.at(childrenBufferPos));
        newItem->setParent(rootItem);
        rootItem->addChild(newItem);
    }

    // Loop through all items
    for (ObjectTreeItemData* itemData : getItemsData()) {
        // If it's root, skip (we already did root, as it has to be treated differently)
        if (itemData == rootItem->getData())
            continue;

        /* If the object is dead, the UpdateObjectTreeClbk won't be called, because Get() can't get anything.
           Dead object should be checked, because they must not have children */
        if (!itemData->isAlive()) {
            for (ObjectTreeItem* sharedItem : itemData->getItemsWithThisData()) {
                for (ObjectTreeItem* childOfReference : sharedItem->getChildren())
                    deleteObjectTreeItem(childOfReference);
                sharedItem->getChildren().clear();
            }
            continue;
        }

        // Get the object is not a solid, go through its children with UpdateObjectTreeClbk
        if (!itemData->isDrawable()) {
            UpdateObjectTreeClbk callback(itemData, this);
            database->Get(itemData->getName().toUtf8().data(), callback);
        }
    }

    // If the item data is not referenced by any item, delete it
    for (ObjectTreeItemData* itemData : getItemsData()) {
        if (itemData->getItemsWithThisData().isEmpty()) {
            getItemsData().remove(itemData->getName());
            delete itemData;
        }
    }

    // Temporary solution to rebuild the objectTreeWidget
    document->getObjectTreeWidget()->update();
    document->getObjectTreeWidget()->refreshItemTextColors();
    document->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
    document->getViewportGrid()->forceRerenderAllViewports();
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


void ObjectTree::deleteObjectTreeItem(ObjectTreeItem* item) {
    // Recursively delete all children
    for (ObjectTreeItem* child : item->getChildren())
        deleteObjectTreeItem(child);

    // Delete item
    getItems().remove(item->getObjectId());
    delete item;
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
            for (ObjectTreeItem* parentItemChild : parentItem->getChildren()) {
                if (parentItemChild->isAlive() && parentItemChild->getVisibilityState() != ObjectTreeItem::FullyVisible) {
                    parentItem->setVisibilityState(ObjectTreeItem::SomeChildrenVisible);
                    break;
                }
            }
            parentItem = parentItem->getParent();
        }

        // Then we go down in the tree and make all children FullyVisible
        traverseSubTree(item, false, [](ObjectTreeItem* item) {
            if (item->isAlive())
                item->setVisibilityState(ObjectTreeItem::FullyVisible);
            return true;
        });
    } else {
        item->setVisibilityState(ObjectTreeItem::Invisible);

        // First we go up in the tree and make the necessary changes
        ObjectTreeItem* parentItem = item->getParent();
        while (!parentItem->isRoot()) {
            // If all children of the ancestor are Invisible after the change ancestor's visibility should be Invisible
            parentItem->setVisibilityState(ObjectTreeItem::Invisible);

            // But if there is a not Invisible child it should be SomeChildrenVisible
            for (ObjectTreeItem* parentItemChild : parentItem->getChildren()) {
                if (parentItemChild->isAlive() && parentItemChild->getVisibilityState() != ObjectTreeItem::Invisible) {
                    parentItem->setVisibilityState(ObjectTreeItem::SomeChildrenVisible);
                    break;
                }
            }
            parentItem = parentItem->getParent();
        }

        // Then we go down in the tree and make all children Invisible
        traverseSubTree(item, false, [](ObjectTreeItem *item) {
            if (item->isAlive())
                item->setVisibilityState(ObjectTreeItem::Invisible);
            return true;
        });
    }
}


size_t ObjectTree::addTopObject(QString name) {
    ObjectTreeItem *topLevelItem = addNewObjectTreeItem(name);
    ObjectTreeItem *rootItem = getRootItem();
    topLevelItem->setParent(rootItem);
    rootItem->addChild(topLevelItem);
    // Get top level object and loop through his children with callback
    BuildObjectTreeClbk callback(this);
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


void ObjectTree::cmdExecutionStarted() {
    cmdBeingExecuted = true;
}


void ObjectTree::cmdExecutionEnded() {
    if (!cmdBeingExecuted)
        return;

    cmdBeingExecuted = false;

    /* If there aren't any queued signals, this method should call updateObjectTree.
       Else, updateObjectTree will be called by the last queued signal */
    if (queuedSignals == 0)
        updateObjectTree();
}


// ---------- OBJECT TREE SIGNALS AND SLOTS ----------

void ObjectTree::queueAddObjectHandler(QString objectName) {
    ++queuedSignals;
    emit addObjectHandlerSignal(objectName);
}


void ObjectTree::queueModifyObjectHandler(QString objectName) {
    ++queuedSignals;
    emit modifyObjectHandlerSignal(objectName);
}


void ObjectTree::queueRemoveObjectHandler(QString objectName) {
    ++queuedSignals;
    emit removeObjectHandlerSignal(objectName);
}


void ObjectTree::addObjectHandler(QString objectName) {
    // Create itemData if it doesn't exist
    ObjectTreeItemData* newItemData;;
    QHash<QString, ObjectTreeItemData*>::const_iterator it = getItemsData().find(objectName);
    if (it == getItemsData().end()) {
        newItemData = new ObjectTreeItemData(objectName);
        getItemsData().insert(objectName, newItemData);
    }

    // Call the modify object handler to retrieve information about the created object
    modifyObjectHandler(objectName);
}


void ObjectTree::modifyObjectHandler(QString objectName) {
    // Get all the informations for the itemData
    ObjectTreeItemData *itemData = getItemsData()[objectName];
    database->Get(objectName.toUtf8().data(), [&itemData](const BRLCAD::Object& object) {
        itemData->setIsAliveFlag(true);
        if (const BRLCAD::Combination* combination = dynamic_cast<const BRLCAD::Combination*>(&object)) {
            if (combination->HasColor()) {
                itemData->getColorInfo().red = combination->Red();
                itemData->getColorInfo().green = combination->Green();
                itemData->getColorInfo().blue = combination->Blue();
                itemData->getColorInfo().hasColor = true;
            }
        } else
            itemData->setIsDrawableFlag(true);
    });

    // If this is the last queued signal, call updateObjectTree
    if (--queuedSignals == 0)
        updateObjectTree();
}


void ObjectTree::removeObjectHandler(QString objectName) {
    // Clear itemData informations
    ObjectTreeItemData *itemData = getItemsData()[objectName];
    itemData->getChildrenOps().clear();
    itemData->setIsAliveFlag(false);
    itemData->setIsDrawableFlag(false);
    itemData->getColorInfo() = {0, 0, 0, false};

    // Make the ObjectTreeItems of the itemData to delete Invisible
    for (ObjectTreeItem* itemWithThisData : itemData->getItemsWithThisData())
        changeVisibilityState(itemWithThisData->getObjectId(), false);

    // If this is the last queued signal, call updateObjectTree
    if (--queuedSignals == 0)
        updateObjectTree();
}
