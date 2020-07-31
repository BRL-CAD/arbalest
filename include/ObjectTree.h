
#ifndef RT3_OBJECTTREE_H
#define RT3_OBJECTTREE_H

#include <QString>
#include <QHash>
#include <QSet>
#include <QVector>
#include "brlcad/MemoryDatabase.h"


class ObjectTree {
public:
    ObjectTree(BRLCAD::MemoryDatabase* database);


	// getters	
    inline BRLCAD::MemoryDatabase* getDatabase() const
    {
	    return database;
    }

    inline QHash<int, QVector<int>>& getTree()
    {
        return tree;
    }

    inline QHash<int, QString>& getNameMap()
    {
        return nameMap;
    }
	
    inline QHash<int, QString>& getFullNameMap()
    {
        return fullNameMap;
    }

    inline QSet<int>& getSolidObjectIds()
    {
        return solidObjectIds;
    }
    inline QVector<int>& getActiveDisplayListIds()
    {
        return activeDisplayListIds;
    }

    inline int getRootObjectId()
    {
        return 0;
    }

    int lastAllocatedId = 0;
	
private:
    BRLCAD::MemoryDatabase* database;
	
    QHash<int, QVector<int>> tree;
    QHash<int, QString> nameMap;
    QHash<int, QString> fullNameMap;
    QSet<int> solidObjectIds;
    QVector<int> activeDisplayListIds;
};

#endif
