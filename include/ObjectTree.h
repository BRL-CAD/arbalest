/*                          O B J E C T T R E E . H
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
/** @file ObjectTree.h
 * 
 *  Generates and stores the object tree of a database.
 *
 *  The ObjectTree is composed of:
 *      - a QHash where a (unique) name is mapped to a ObjectTreeItemData
 *      - a QHash where a (unique) object id is mapped to a ObjectTreeItem
 *
 *  ObjectTreeItemData is the representation of an unique object in the database. As such, for example,
 *  all occurrences in a database of "obj":
 *      - have the same name,
 *      - have the same children operations (all children items of an item with this item data will be
 *        treated with the same operations),
 *      - are all either drawable or not (solid or combination),
 *      - are all either alive or dead (killed).
 *
 *  bjectTreeItem is the representation of an object in the database tree. As such, for example, all
 *  occurrences in a database of "obj":
 *      - will have different children items (which share the same item data), for example:
 *            obj
 *             |_____ u sph1.s  (different item from the other sph1.s, but same item data)
 *             |_____ - sph2.s  (different item from the other sph2.s, but same item data)
 *            ...
 *            obj
 *             |_____ u sph1.s  (different item from the other sph1.s, but same item data)
 *             |_____ - sph2.s  (different item from the other sph2.s, but same item data)
 *      - might have a different parent, for example:
 *            comb1.c
 *               |_____ u obj
 *              ...
 *            comb2.c
 *               |_____ u obj
 *      - might have a different visibility state
 *
 *  So with the use of these 2 classes, we can represent what is SHARED across ALL occurrences in a
 *  database of an object through ObjectTreeItemData, and what is UNIQUE FOR EACH occurrences in a
 *  database of an object through ObjectTreeItem.
 */

#ifndef RT3_OBJECTTREE_H
#define RT3_OBJECTTREE_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>
#include <brlcad/Database/Combination.h>
#include "brlcad/Database/ConstDatabase.h"
#include "brlcad/Database/MemoryDatabase.h"
#include "Utils.h"


class ObjectTreeItem;


class ObjectTreeItemData {
public:
    ObjectTreeItemData(QString name) : name(name) {};

    void addOp(BRLCAD::Combination::ConstTreeNode::Operator op);

    // Setters
    void setIsAliveFlag(bool isAlive) {
        isAliveFlag = isAlive;
    }

    void setIsDrawableFlag(bool isDrawable) {
        isDrawableFlag = isDrawable;
    }

    // Getters
    QVector<ObjectTreeItem *>& getItemsWithThisData(void) {
        return itemsWithThisData;
    }

    QString getName(void) {
        return name;
    }

    QVector<BRLCAD::Combination::ConstTreeNode::Operator>& getChildrenOps(void) {
        return childrenOps;
    }

    bool isAlive(void) {
        return isAliveFlag;
    }

    bool isDrawable(void) {
        return isDrawableFlag;
    }

    ColorInfo& getColorInfo(void) {
        return colorInfo;
    }

private:
    QVector<ObjectTreeItem *> itemsWithThisData;

    QString name;
    QVector<BRLCAD::Combination::ConstTreeNode::Operator> childrenOps;

    bool isAliveFlag = false;
    bool isDrawableFlag = false;
    ColorInfo colorInfo = {0, 0, 0, false};
};


class ObjectTreeItem {
public:
    enum VisibilityState{
        Invisible,
        SomeChildrenVisible,
        FullyVisible,
    };

    ObjectTreeItem(ObjectTreeItemData* data, size_t uniqueObjectId);
    ~ObjectTreeItem();

    void addChild(ObjectTreeItem *itemParent);

    bool isRoot(void);

    QString getPath(void);

    // Getters
    ObjectTreeItemData *getData(void) {
        return data;
    }

    size_t getObjectId(void) {
        return uniqueObjectId;
    }

    ObjectTreeItem *getParent(void) {
        return parent;
    }

    QVector<ObjectTreeItem *>& getChildren(void) {
        return children;
    }

    VisibilityState getVisibilityState(void) {
        return visibilityState;
    }

    // Getters for data
    QVector<ObjectTreeItem *>& getItemsWithSameData(void) {
        return data->getItemsWithThisData();
    }

    QString getName(void) {
        return data->getName();
    }

    QVector<BRLCAD::Combination::ConstTreeNode::Operator>& getChildrenOps(void) {
        return data->getChildrenOps();
    }

    bool isAlive(void) {
        return data->isAlive();
    }

    bool isDrawable(void) {
        return data->isDrawable();
    }

    ColorInfo& getColorInfo(void) {
        return data->getColorInfo();
    }

    // Setters
    void setParent(ObjectTreeItem *itemParent) {
        parent = itemParent;
    }

    void setVisibilityState(VisibilityState newVisibilityState) {
        visibilityState = newVisibilityState;
    }

private:
    ObjectTreeItemData *data;
    size_t uniqueObjectId;

    ObjectTreeItem *parent;
    QVector<ObjectTreeItem *> children;

    VisibilityState visibilityState = Invisible;
};


class ObjectTree : public QObject {
    Q_OBJECT
public:
    ObjectTree(BRLCAD::MemoryDatabase* database);
    ~ObjectTree();

    size_t lastAllocatedId = 0;

    void traverseSubTree(ObjectTreeItem *rootOfSubTree, const bool traverseRoot, const std::function<bool(ObjectTreeItem*)>& callback);

    void changeVisibilityState(size_t objectId, bool visible);

    // Given a name, create a new item and assign its item data (if the name is new, create a new item data)
    ObjectTreeItem *addNewObjectTreeItem(QString name);

    void deleteObjectTreeItem(ObjectTreeItem* item);

    size_t addTopObject(QString name);

    void updateObjectTree();

    static void databaseChangeHandler(const char* objectName, BRLCAD::ConstDatabase::ChangeType changeType);

    void cmdExecutionStarted();
    void cmdExecutionEnded();

    void queueAddObjectHandler(QString objectName);
    void queueModifyObjectHandler(QString objectName);
    void queueRemoveObjectHandler(QString objectName);

    // getters
    BRLCAD::MemoryDatabase* getDatabase() const
    {
	    return database;
    }

    QHash<size_t, ObjectTreeItem*> &getItems() {
        return items;
    }

    QHash<QString, ObjectTreeItemData*> &getItemsData() {
        return itemsData;
    }

    ObjectTreeItem* getRootItem() {
        return getItems()[0];
    }

    bool isCmdBeingExecuted() {
        return cmdBeingExecuted;
    }
	
private:
    BRLCAD::MemoryDatabase* database;

    class BuildObjectTreeClbk {
    public:
        BuildObjectTreeClbk(ObjectTree* objectTree) : objectTree(objectTree) {}
        void operator()(const BRLCAD::Object& object);
    private:
        void traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node);

        ObjectTree* objectTree = nullptr;
        ObjectTreeItem* currItem = nullptr;
        BRLCAD::Combination::ConstTreeNode::Operator currOp = BRLCAD::Combination::ConstTreeNode::Null;
    };

    class UpdateObjectTreeClbk {
    public:
        UpdateObjectTreeClbk(ObjectTreeItemData* currItemData, ObjectTree* objectTree) : currItemData(currItemData), objectTree(objectTree) {}
        void operator()(const BRLCAD::Object& object);
    private:
        void traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node);

        ObjectTree* objectTree = nullptr;
        ObjectTreeItemData* currItemData = nullptr;

        QVector<QString> childrenBuffer = {};
        QVector<BRLCAD::Combination::ConstTreeNode::Operator> childrenOpBuffer = {};
    };

    BRLCAD::ConstDatabase::ChangeSignalHandler databaseChangeHandlerVar = this->databaseChangeHandler;

    bool cmdBeingExecuted = false;

    size_t queuedSignals = 0;

    // All items
    QHash<size_t, ObjectTreeItem*> items;

    // Unique name to item data
    QHash<QString, ObjectTreeItemData*> itemsData;

signals:
    void addObjectHandlerSignal(QString objectName);
    void modifyObjectHandlerSignal(QString objectName);
    void removeObjectHandlerSignal(QString objectName);

public slots:
    void addObjectHandler(QString objectName);
    void modifyObjectHandler(QString objectName);
    void removeObjectHandler(QString objectName);
};

#endif
