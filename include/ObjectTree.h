
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

    const QString rootName = "/";

	// getters	
    BRLCAD::MemoryDatabase* getDatabase() const
    {
	    return database;
    }

    QHash<QString, QVector<QString>>& getTree()
    {
	    return tree;
    }

	QVector<QString> & getRoot()
    {
        return tree["/"];
    }

private:
    BRLCAD::MemoryDatabase* database;
    QHash<QString, QVector<QString>> tree;
	
};

#endif
