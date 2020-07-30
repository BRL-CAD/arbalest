
#ifndef RT3_OBJECTTREE_H
#define RT3_OBJECTTREE_H

#include <QString>
#include <QHash>
#include <QVector>
#include <brlcad/ConstDatabase.h>

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

    int lastAllocatedId = 0;
	
private:
    BRLCAD::MemoryDatabase* database;
	
    QHash<int, QVector<int>> tree;
    QHash<int, QString> nameMap;
};

#endif
