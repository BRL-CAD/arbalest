/*                   O B J E C T T R E E W I D G E T . C P P
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
/** @file ObjectTreeWidget.cpp */

#include <brlcad/Database/Combination.h>
#include "Globals.h"
#include "ObjectTreeRowButtons.h"

#include "ObjectTreeWidget.h"


ObjectTreeWidget::ObjectTreeWidget(Document* document, QWidget* parent) : document(document) {
    setTextColor();

	setHeaderHidden(true);
	setMouseTracking(true);
	setColumnCount(1);

	build(0);

    ObjectTreeRowButtons *visibilityButton = new ObjectTreeRowButtons(document->getObjectTree(), this);
    setItemDelegateForColumn(0, visibilityButton);

    connect(this, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* current, QTreeWidgetItem* previous) {
        selectionChanged((size_t)(current->data(0, Qt::UserRole).toLongLong()));
        // Qt changes foreground color for selected items. We don't want it changed
        setStyleSheet("ObjectTreeWidget::item:selected { color: " + current->foreground(0).color().name() + ";}");
    });

    connect(visibilityButton, &ObjectTreeRowButtons::visibilityButtonClicked, this, [this](size_t objectId) {
        switch (this->document->getObjectTree()->getItems()[objectId]->getVisibilityState()) {
            case ObjectTreeItem::Invisible:
            case ObjectTreeItem::SomeChildrenVisible:
                this->document->getObjectTree()->changeVisibilityState(objectId, true);
                break;
            case ObjectTreeItem::FullyVisible:
                this->document->getObjectTree()->changeVisibilityState(objectId, false);
                break;
        }
        this->document->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        this->document->getViewportGrid()->forceRerenderAllViewports();
        refreshItemTextColors();
    });

    connect(visibilityButton, &ObjectTreeRowButtons::centerButtonClicked, this, [this](size_t objectId) {
        this->document->getViewport()->getCamera()->centerView(objectId);
        this->document->getViewportGrid()->forceRerenderAllViewports();
        refreshItemTextColors();
    });

    refreshItemTextColors();
}


void ObjectTreeWidget::build(const size_t objectId, QTreeWidgetItem* parent) {
    if (objectId == 0)
        clear();

    ObjectTreeItem *objTreeItem = document->getObjectTree()->getItems()[objectId];

	QTreeWidgetItem* item;

	if (objectId != 0) {
        item = new QTreeWidgetItem();
        objectIdTreeWidgetItemMap[objectId] = item;
        item->setText(0, objTreeItem->getName());
        item->setData(0, Qt::UserRole, (qulonglong)objectId);

        item->setDisabled(!objTreeItem->isAlive());

        if (parent != nullptr)
            parent->addChild(item);
        else
            addTopLevelItem(item);
    }

	for (ObjectTreeItem *objTreeItemChild : objTreeItem->getChildren())
		build(objTreeItemChild->getObjectId(), (objectId != 0) ? item : nullptr);
}


void ObjectTreeWidget::destroy(const size_t objectId) {
    QHash<size_t, QTreeWidgetItem*>::iterator it = objectIdTreeWidgetItemMap.find(objectId);
    if (it == objectIdTreeWidgetItemMap.end())
        return;

    QTreeWidgetItem* item = it.value();
    item->setSelected(false);

    if (item == currentItem())
        setCurrentItem(nullptr);

    // If item is not top-level, remove itself from parent's children
    QTreeWidgetItem* parent = item->parent();
    if (parent)
        parent->removeChild(item);

    // Remove from objectIdTreeWidgetItemMap the children of the item to delete
    traverseSubTree(item, true, [this](QTreeWidgetItem* item) {
        size_t id = (size_t)(item->data(0, Qt::UserRole).toLongLong());
        this->objectIdTreeWidgetItemMap.remove(id);
        return true;
    });

    delete item;
}


void ObjectTreeWidget::update() {
    QHash<size_t, ObjectTreeItem*>& objTreeItems = document->getObjectTree()->getItems();

    // Queue items that need to be destroyed
    QVector<QTreeWidgetItem*> itemsToDelete = {};
    int i = 0;
    while (i < topLevelItemCount()) {
        QTreeWidgetItem* topItem = topLevelItem(i);

        size_t id = (size_t)(topItem->data(0, Qt::UserRole).toLongLong());
        if (objTreeItems.find(id) == objTreeItems.end()) {
            // If the top-level needs to be destroyed, queue it
            itemsToDelete.append(topItem);
        } else {
            // Else, look if a child of the top-level item needs to be destroyed
            traverseSubTree(topItem, false, [&itemsToDelete, &objTreeItems](QTreeWidgetItem* item) {
                size_t id = (size_t)(item->data(0, Qt::UserRole).toLongLong());
                if (objTreeItems.find(id) == objTreeItems.end()) {
                    itemsToDelete.append(item);
                    return false;
                }
                return true;
            });
        }

        ++i;
    }

    // Destroy queued items
    for (QTreeWidgetItem* itemToDelete : itemsToDelete) {
        size_t id = (size_t)(itemToDelete->data(0, Qt::UserRole).toLongLong());
        destroy(id);
    }

    // Update the ObjectTreeWidget
    document->getObjectTree()->traverseSubTree(document->getObjectTree()->getRootItem(), false, [this](ObjectTreeItem* objTreeItem) {
        size_t objTreeItemId = objTreeItem->getObjectId();

        // If objTreeItem is new, build corresponding item
        QHash<size_t, QTreeWidgetItem*>::iterator it = this->objectIdTreeWidgetItemMap.find(objTreeItemId);
        if (it == this->objectIdTreeWidgetItemMap.end()) {
            this->build(objTreeItemId);
            // Return false, so that we don't got through the subtree of the new item (it will be handled by build)
            return false;
        }

        // Else, update corresponding item
        QTreeWidgetItem* item = this->objectIdTreeWidgetItemMap[objTreeItemId];
        item->setDisabled(!objTreeItem->isAlive());

        return true;
    });
}


void ObjectTreeWidget::select(QString selected) {
    QStringList regionName = selected.split("/");
    QString path = "/" + regionName[1];
    int regionNameSize = regionName.size();
    int regionNameIndex = 1;
    int mapSize = document->getObjectTree()->getItems().size();

    for (size_t objectId = 1; objectId < mapSize; ++objectId) {
        if (document->getObjectTree()->getItems()[objectId]->getPath() == path) {
            if (regionNameIndex == regionNameSize - 1) {
                objectIdTreeWidgetItemMap[objectId]->setSelected(true);
                document->getProperties()->bindObject(objectId);
                break;
            }
            expandItem(objectIdTreeWidgetItemMap[objectId]);
            ++regionNameIndex;
            path = path + "/" + regionName[regionNameIndex];
        }
    }
}


void ObjectTreeWidget::traverseSubTree(QTreeWidgetItem *rootOfSubTree, bool traverseRoot, const std::function<bool(QTreeWidgetItem*)>& callback) {
	if (traverseRoot) callback(rootOfSubTree);
    int i = 0;
	while (i < rootOfSubTree->childCount()) {
        QTreeWidgetItem* item = rootOfSubTree->child(i);
        ++i;
		if (!callback(item)) continue;
        if (!item->childCount() == 0) traverseSubTree(item, false, callback);
	}
}


void ObjectTreeWidget::refreshItemTextColors() {
    document->getObjectTree()->traverseSubTree(document->getObjectTree()->getRootItem(), false, [this](ObjectTreeItem* item) {
        if (!item->isAlive()) {
            this->objectIdTreeWidgetItemMap[item->getObjectId()]->setForeground(0, QBrush(colorDead));
            return true;
        }

        switch (item->getVisibilityState()) {
            case ObjectTreeItem::Invisible:
                this->objectIdTreeWidgetItemMap[item->getObjectId()]->setForeground(0, QBrush(colorInvisible));
                break;
            case ObjectTreeItem::SomeChildrenVisible:
                this->objectIdTreeWidgetItemMap[item->getObjectId()]->setForeground(0, QBrush(colorSomeChildrenVisible));
                break;
            case ObjectTreeItem::FullyVisible:
                this->objectIdTreeWidgetItemMap[item->getObjectId()]->setForeground(0, QBrush(colorFullVisible));
                break;
        }

        return true;
    });

    // Qt changes foreground color for selected items. We don't want it changed
    if (currentItem())
        setStyleSheet("ObjectTreeWidget::item:selected { color: " + currentItem()->foreground(0).color().name() + ";}");
}


void ObjectTreeWidget::setTextColor() {
    colorFullVisible = QColor(Globals::theme->process("$Color-FullyVisibleObjectText"));
    colorSomeChildrenVisible = QColor(Globals::theme->process("$Color-SomeChildrenVisibleObjectText"));
    colorInvisible = QColor(Globals::theme->process("$Color-InvisibleObjectText"));
    colorDead = QColor(Globals::theme->process("$Color-DeadObjectText"));
}
