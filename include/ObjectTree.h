
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
       - have the same children (which are treated with the same operations),
       - are all either drawable or not,
       - are all either alive or dead (killed).

   ObjectTreeItem is the representation of an object in the database tree. As such, for example, all
   occurrences in a database of "obj":
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

    void addChild(ObjectTreeItem *itemParent);

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

    QVector<ObjectTreeItem *>& getChildren(void) {
        return children;
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

private:
    QVector<ObjectTreeItem *> itemsWithThisData;

    QString name;
    QVector<ObjectTreeItem *> children;
    QVector<BRLCAD::Combination::ConstTreeNode::Operator> childrenOps;

    bool isAliveFlag = false;
    bool isDrawableFlag = false;
};


class ObjectTreeItem {
public:
    enum VisibilityState{
        Invisible,
        SomeChildrenVisible,
        FullyVisible,
    };

    ObjectTreeItem(ObjectTreeItemData* data);

    bool isRoot(void);

    QString getPath(void);

    // Getters
    ObjectTreeItem *getParent(void) {
        return parent;
    }

    ObjectTreeItemData *getData(void) {
        return data;
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

    QVector<ObjectTreeItem *>& getChildren(void) {
        return data->getChildren();
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

    // Setters
    void setParent(ObjectTreeItem *itemParent) {
        parent = itemParent;
    }

    void setVisibilityState(VisibilityState newVisibilityState) {
        visibilityState = newVisibilityState;
    }

private:
    ObjectTreeItemData *data;

    ObjectTreeItem *parent;

    VisibilityState visibilityState = Invisible;
};

/*
 * Generates and stores the object tree by reading a database.
 * This class is used by ObjectTreeWidget, GeometryRenderer etc. throughout the application a lot.
 * This class also contains several data structures needed for rendering the tree.
 * BRLCAD already provides options for traversing the database.
 * But BRLCAD traverses through the entire bool tree. Also we cannot randomly access nodes in the tree.
 * Therefore in this class we use the BRLCAD methods to copy and create out own tree.
 * 
 *
 * We assign an integer id to each object to identify it. Same object can have multiple ids if they appear
 * multiple times in the tree like all.g\orb and ball.g\orb
 */

class ObjectTree {
public:
    enum VisibilityState{
        Invisible,
        SomeChildrenVisible,
        FullyVisible,
    };

    ObjectTree(BRLCAD::MemoryDatabase* database);

    int lastAllocatedId = 0;

    void traverseSubTree(int rootOfSubTreeId, bool traverseRoot, const std::function<bool(int)>&);

    void changeVisibilityState(int objectId, bool visible);
    void buildColorMap(int rootObjectId);
    int addTopObject(QString name);

        // getters
    BRLCAD::MemoryDatabase* getDatabase() const
    {
	    return database;
    }

    QHash<int, QVector<int>>& getChildren()
    {
        return objectIdChildrenObjectIdsMap;
    }

    QHash<int, int>& getParent()
    {
        return objectIdParentObjectIdMap;
    }

    QHash<int, QString>& getNameMap()
    {
        return nameMap;
    }

    QHash<int, QString>& getFullPathMap()
    {
        return fullPathMap;
    }
	
    QHash<int, ColorInfo>& getColorMap()
    {
        return colorMap;
    }

    QSet<int>& getDrawableObjectIds()
    {
        return drawableObjectIds;
    }

    QHash<int, VisibilityState> &getObjectVisibility() {
        return objectIdVisibilityStateMap;
    }
	
private:
    BRLCAD::MemoryDatabase* database;
	
	// this class is used for traversing the MemoryDatabase and produce the tree
    class ObjectTreeCallback {
    public:
        ObjectTreeCallback(ObjectTree* objectTree, QString& objectName, const int& parentObjectId) :
            objectTree(objectTree),
            objectName(objectName),
            parentObjectId(parentObjectId),
            currentObjectPath(objectTree->fullPathMap[parentObjectId] + "/" + objectName) {}
        void operator()(const BRLCAD::Object& object);
    private:
        int objectId = -1;
        ObjectTree* objectTree = nullptr;
        QString objectName;
        const int& parentObjectId;
    	QString currentObjectPath;
        QVector<int>* childrenNames = nullptr;
        void traverseSubTree(const BRLCAD::Combination::ConstTreeNode& node) const; //traverse the boolean tree of the MemoryDatabase
    };

    // Stores the object tree in  {parent's object id (key), children's object ids (value)} format
    QHash<int, QVector<int>>    objectIdChildrenObjectIdsMap;

    // Stores the object tree in  { object id (key), parent's object id (value)} format
    QHash<int, int>    objectIdParentObjectIdMap;

	// Object id to it's name mapping
    QHash<int, QString>         nameMap;

    // Object id to it's full path mapping
    QHash<int, QString>         fullPathMap;

    // Object id to it's full path mapping
    QHash<int, ColorInfo>         colorMap;

    // Get all objects that are not combinations. (ie. these are also the objects that can be drawn) //todo _GLOBAL?
    QSet<int>                   drawableObjectIds;


    QHash<int, VisibilityState>             objectIdVisibilityStateMap;


    // All items (except rootItem)
    QHash<unsigned int, ObjectTreeItem*> items;

    // Unique name to item data
    QHash<QString, ObjectTreeItemData*> itemsData;
};

#endif
