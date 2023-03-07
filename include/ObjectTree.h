
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


#include "Utils.h"

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
};

#endif
