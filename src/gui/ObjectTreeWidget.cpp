/*                    O B J E C T S T R E E V I E W . C P P
 * BRL-CAD
 *
 * Copyright (c) 2018 United States Government as represented by
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
/** @file ObjectTreeWidget.cpp
 *
 */

#include <brlcad/Database/Combination.h>
#include "ObjectTreeWidget.h"
#include <QHBoxLayout>
#include <QtCore/QtCore>
#include <include/ObjectTreeRowButtons.h>
#include "Globals.h"

ObjectTreeWidget::ObjectTreeWidget(Document* document, QWidget* parent) : document(document)
{
    setTextColor();

	this->setHeaderHidden(true);
	setColumnCount(1);
	setMouseTracking(true);

	build(0);

    ObjectTreeRowButtons *visibilityButton = new ObjectTreeRowButtons(document->getObjectTree(), this);
    setItemDelegateForColumn(0, visibilityButton);

    connect(this,&QTreeWidget::currentItemChanged,this,[this](QTreeWidgetItem *current, QTreeWidgetItem *previous){
        selectionChanged (current->data(0, Qt::UserRole).toInt());
        // Qt changes foreground color for selected items. We don't want it changed
        setStyleSheet("ObjectTreeWidget::item:selected { color: "+current->foreground(0).color().name()+";}");
    });
    connect(visibilityButton, &ObjectTreeRowButtons::visibilityButtonClicked, this, [this](unsigned int objectId){
        switch(this->document->getObjectTree()->getItems()[objectId]->getVisibilityState()){
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

    connect(visibilityButton, &ObjectTreeRowButtons::centerButtonClicked, this, [this](unsigned int objectId){
        this->document->getViewport()->getCamera()->centerView(objectId);
        this->document->getViewportGrid()->forceRerenderAllViewports();
        refreshItemTextColors();
    });
    refreshItemTextColors();
}

void ObjectTreeWidget::build(const unsigned int objectId, QTreeWidgetItem* parent) {
    ObjectTreeItem *objTreeItem = document->getObjectTree()->getItems()[objectId];

	QTreeWidgetItem* item;

	if (objectId != 0) {
        item = new QTreeWidgetItem();
        objectIdTreeWidgetItemMap[objectId] = item;
        item->setText(0, objTreeItem->getName());
        item->setData(0, Qt::UserRole, objectId);

        if (parent != nullptr)
            parent->addChild(item);
        else
            addTopLevelItem(item);
    }

	for (ObjectTreeItem *objTreeItemChild : objTreeItem->getChildren())
		build(objTreeItemChild->getObjectId(), (objectId != 0) ? item: nullptr);
}

void ObjectTreeWidget::select(QString selected) {
    QStringList regionName = selected.split("/");
    QString path = "/" + regionName[1];
    int regionNameSize = regionName.size();
    int regionNameIndex = 1;
    int mapSize = document->getObjectTree()->getItems().size();

    for (unsigned int objectId = 1; objectId < mapSize; ++objectId) {
        if (document->getObjectTree()->getItems()[objectId]->getPath() == path) {
            if (regionNameIndex == regionNameSize - 1) {
                objectIdTreeWidgetItemMap[objectId]->setSelected(true);
                document->getProperties()->bindObject(objectId);
                break;
            }
            this->expandItem(objectIdTreeWidgetItemMap[objectId]);
            ++regionNameIndex;
            path = path + "/" + regionName[regionNameIndex];
        }
    }
}


const QHash<unsigned int, QTreeWidgetItem *> &ObjectTreeWidget::getObjectIdTreeWidgetItemMap() const {
    return objectIdTreeWidgetItemMap;
}

void ObjectTreeWidget::refreshItemTextColors() {
    document->getObjectTree()->traverseSubTree(0, false, [this](unsigned int objectId) {
        switch (document->getObjectTree()->getItems()[objectId]->getVisibilityState()) {
            case ObjectTreeItem::Invisible:
                objectIdTreeWidgetItemMap[objectId]->setForeground(0, QBrush(colorInvisible));
                break;
            case ObjectTreeItem::SomeChildrenVisible:
                objectIdTreeWidgetItemMap[objectId]->setForeground(0, QBrush(colorSomeChildrenVisible));
                break;
            case ObjectTreeItem::FullyVisible:
                objectIdTreeWidgetItemMap[objectId]->setForeground(0, QBrush(colorFullVisible));
                break;
        }

        return true;
    });

    // Qt changes foreground color for selected items. We don't want it changed
    if (currentItem()){
        setStyleSheet("ObjectTreeWidget::item:selected { color: "+currentItem()->foreground(0).color().name()+";}");
    }
}

void ObjectTreeWidget::setTextColor() {
    colorFullVisible = QColor(Globals::theme->process("$Color-FullyVisibleObjectText"));
    colorSomeChildrenVisible = QColor(Globals::theme->process("$Color-SomeChildrenVisibleObjectText"));
    colorInvisible = QColor(Globals::theme->process("$Color-InvisibleObjectText"));
}
