
#ifndef RT3_OBJECTTREE_H
#define RT3_OBJECTTREE_H

#include <QString>
#include <QHash>
#include <QSet>
#include <QVector>
#include "brlcad/Database/MemoryDatabase.h"
#include <brlcad/Database/Combination.h>
#include <functional>
#include <set>
#include <QString>
#include <brlcad/CommandString/CommandString.h>
#include "Utils.h"


/* Generates and stores the object tree of a database.

   The ObjectTree is composed of:
       - a QHash where a (unique) name is mapped to a ObjectTreeItemData
       - a QSet of all ObjectTreeItem that make up the tree
    
   ObjectTreeItemData is the representation of an unique object in the database. As such, for example,
   all occurrences in a database of "obj":
       - have the same name,
       - have the same children operations (all children items of an item with this item data will be
         treated with the same operations),
       - are all either drawable or not (solid or combination),
       - are all either alive or dead (killed).

   ObjectTreeItem is the representation of an object in the database tree. As such, for example, all
   occurrences in a database of "obj":
       - will have different children items (which share the same item data), for example:
             obj
              |_____ u sph1.s  (different item from the other sph1.s, but same item data)
              |_____ - sph2.s  (different item from the other sph2.s, but same item data)
             ...
             obj
              |_____ u sph1.s  (different item from the other sph1.s, but same item data)
              |_____ - sph2.s  (different item from the other sph2.s, but same item data)
       - might have a different parent, for example:
             comb1.c
                |_____ u obj
               ...
             comb2.c
                |_____ u obj
       - might have a different visibility state:

   So with the use of these 2 classes, we can represent what is SHARED across ALL occurrences in a database
   of an object through ObjectTreeItemData, and what is UNIQUE FOR EACH occurrences in a database of an
   object through ObjectTreeItem */


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

    ObjectTreeItem(ObjectTreeItemData* data, unsigned int uniqueObjectId);

    void addChild(ObjectTreeItem *itemParent);

    bool isRoot(void);

    QString getPath(void);

    // Getters
    ObjectTreeItemData *getData(void) {
        return data;
    }

    unsigned int getObjectId(void) {
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
    unsigned int uniqueObjectId;

    ObjectTreeItem *parent;
    QVector<ObjectTreeItem *> children;

    VisibilityState visibilityState = Invisible;
};


class ObjectTree {
public:
    ObjectTree(BRLCAD::MemoryDatabase* database);

    unsigned int lastAllocatedId = 0;

    void traverseSubTree(ObjectTreeItem *rootOfSubTree, bool traverseRoot, const std::function<bool(ObjectTreeItem*)>& callback);
    void traverseSubTree(const unsigned rootOfSubTreeId, bool traverseRoot, const std::function<bool(unsigned int)>& callback);

    void changeVisibilityState(unsigned int objectId, bool visible);

    // Given a name, create a new item and assign its item data (if the name is new, create a new item data)
    ObjectTreeItem *addNewObjectTreeItem(QString name);

    unsigned int addTopObject(QString name);

    void printTree(void);

    // getters
    BRLCAD::MemoryDatabase* getDatabase() const
    {
	    return database;
    }

    QHash<unsigned int, ObjectTreeItem*> &getItems() {
        return items;
    }

    QHash<QString, ObjectTreeItemData*> &getItemsData() {
        return itemsData;
    }
	
private:
    BRLCAD::MemoryDatabase* database;

    class objectTreeCallback {
    public:
        objectTreeCallback(ObjectTree* objectTree) : objectTree(objectTree) {}
        void operator()(const BRLCAD::Object& object);

    private:
        void traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node);

        ObjectTree* objectTree = nullptr;
        ObjectTreeItem* currItem = nullptr;
        BRLCAD::Combination::ConstTreeNode::Operator currOp = BRLCAD::Combination::ConstTreeNode::Null;
    };

    // All items
    QHash<unsigned int, ObjectTreeItem*> items;

    // Unique name to item data
    QHash<QString, ObjectTreeItemData*> itemsData;
};

#endif
