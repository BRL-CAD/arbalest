
#ifndef RT3_OBJECTTREE_H
#define RT3_OBJECTTREE_H

#include <QString>
#include <QHash>
#include <QSet>
#include <QVector>
#include "brlcad/MemoryDatabase.h"
#include <brlcad/Combination.h>
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
    ObjectTree(BRLCAD::MemoryDatabase* database);

    int lastAllocatedId = 0;

    void traverseSubTree(int rootOfSubTreeId, const std::function<void(int)>&);

    // builds visibleDisplayListIds from visibleObjectIds and objectIdDisplayListIdMap
    void rebuildVisibleDisplayListIds();

    void changeSubTreeVisibility(int rootOfSubTreeId, bool visible);
	
	// getters	
    BRLCAD::MemoryDatabase* getDatabase() const
    {
	    return database;
    }

    QHash<int, QVector<int>>& getTree()
    {
        return tree;
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

    QHash<int, int>& getObjectIdDisplayListIdMap()
    {
        return objectIdDisplayListIdMap;
    }

    QSet<int>& getDrawableObjectIds()
    {
        return drawableObjectIds;
    }

    std::set<int>& getVisibleObjectIds()
    {
        return visibleObjectIds;
    }

    const QVector<int>& getVisibleDisplayListIds() const
    {
        return visibleDisplayListIds;
    }
	
    QVector<int>& getObjectsToBeDrawnIds()
    {
        return objectsToBeDrawnIds;
    }

    int getRootObjectId() const
    {
        return 0;
    }
	
private:
    BRLCAD::MemoryDatabase* database;
	
	// this class is used for traversing the MemoryDatabase and produce the tree
    class ObjectTreeCallback : public BRLCAD::ConstDatabase::ObjectCallback {
    public:
        ObjectTreeCallback(ObjectTree* objectTree, QString& objectName, const int& parentObjectId) :
            objectTree(objectTree),
            objectName(objectName),
            parentObjectId(parentObjectId),
            currentObjectPath(objectTree->fullPathMap[parentObjectId] + "/" + objectName) {}
        void operator()(const BRLCAD::Object& object) override;
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
    QHash<int, QVector<int>>    tree;

	// Object id to it's name mapping
    QHash<int, QString>         nameMap;

    // Object id to it's full path mapping
    QHash<int, QString>         fullPathMap;

    // Object id to it's full path mapping
    QHash<int, ColorInfo>         colorMap;

	// Contains generated display list alone with corresponding objectId. objectId is the key. displayListId is value.
    QHash<int, int>             objectIdDisplayListIdMap;

    // Get all objects that are not combinations. (ie. these are also the objects that can be drawn) //todo _GLOBAL?
    QSet<int>                   drawableObjectIds;

	// This set contains all visible objects that can drawn. It only contains drawableObjects, so no combinations.
	// Should be updated whenever user makes subtrees (in)visible from ui etc.
    std::set<int>                   visibleObjectIds;

    // Needs to be recalculated whenever visibleObjectIds changes or objects are redrawn (i.e. get a new display list)
    QVector<int>                visibleDisplayListIds;

    // GeometryRenderer should draw in this vector, and generate display lists and then clear() this QVector in its
    // next render() event if there are items in this vector
    // Objects that were changed should be added here to acknowledge it to redraw
    QVector<int>                objectsToBeDrawnIds;

};

#endif
