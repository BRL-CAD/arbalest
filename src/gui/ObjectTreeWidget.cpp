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

#include <brlcad/Combination.h>
#include "ObjectTreeWidget.h"
#include <QHBoxLayout>
#include <QtCore/QtCore>
#include <include/ObjectTreeRowButtons.h>
#include "Globals.h"

ObjectTreeWidget::ObjectTreeWidget(Document* document, QWidget* parent) : document(document)
{

    colorFullVisible = QColor(Globals::theme->process("$Color-FullyVisibleObjectText"));
    colorSomeChildrenVisible = QColor(Globals::theme->process("$Color-SomeChildrenVisibleObjectText"));
    colorInvisible = QColor(Globals::theme->process("$Color-InvisibleObjectText"));

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
    connect(visibilityButton, &ObjectTreeRowButtons::visibilityButtonClicked, this, [this](int objectId){
        switch(this->document->getObjectTree()->getObjectVisibility()[objectId]){
            case ObjectTree::Invisible:
            case ObjectTree::SomeChildrenVisible:
                this->document->getObjectTree()->changeVisibilityState(objectId, true);
                break;
            case ObjectTree::FullyVisible:
                this->document->getObjectTree()->changeVisibilityState(objectId, false);
                break;
        }
        this->document->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        this->document->getDisplayGrid()->forceRerenderAllDisplays();
        refreshItemTextColors();
    });

    connect(visibilityButton, &ObjectTreeRowButtons::centerButtonClicked, this, [this](int objectId){
        this->document->getDisplay()->getCamera()->centerView(objectId);
        this->document->getDisplayGrid()->forceRerenderAllDisplays();
        refreshItemTextColors();
    });
    refreshItemTextColors();
}

void ObjectTreeWidget::build(const int objectId, QTreeWidgetItem* parent)
{
	QTreeWidgetItem* item = nullptr;

	if (objectId != 0) {
        item = new QTreeWidgetItem();
        objectIdTreeWidgetItemMap[objectId] = item;
        item->setText(0,document->getObjectTree()->getNameMap()[objectId]);
        item->setData(0, Qt::UserRole, objectId);

        if (parent != nullptr) {
            parent->addChild(item);
        } else {
            addTopLevelItem(item);
        }
    }

	for (int childObjectId : document->getObjectTree()->getChildren()[objectId])
	{
		build(childObjectId, objectId ? item: nullptr);
	}
}

const QHash<int, QTreeWidgetItem *> &ObjectTreeWidget::getObjectIdTreeWidgetItemMap() const {
    return objectIdTreeWidgetItemMap;
}

void ObjectTreeWidget::refreshItemTextColors() {
    document->getObjectTree()->traverseSubTree(0,false,[this](int objectId){
        switch (document->getObjectTree()->getObjectVisibility()[objectId]){

            case ObjectTree::Invisible:
                objectIdTreeWidgetItemMap[objectId]->setForeground(0, QBrush(colorInvisible));
                break;
            case ObjectTree::SomeChildrenVisible:
                objectIdTreeWidgetItemMap[objectId]->setForeground(0, QBrush(colorSomeChildrenVisible));
                break;
            case ObjectTree::FullyVisible:
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

QStringList ObjectTreeWidget::getSelectedObjects(const Name& name, const Level& level) {
    QStringList ans;
    ObjectTree* objTree = document->getObjectTree();

    // if top, only traverse children of root
    if (level == Level::TOP) {
        for (int childObjectId : objTree->getChildren()[0]) {
            if (objTree->getObjectVisibility()[childObjectId] == ObjectTree::VisibilityState::FullyVisible)
                ans << objTree->getFullPathMap()[childObjectId];
        }
    } 
    
    // otherwise check entire tree
    else {
        document->getObjectTree()->traverseSubTree(0,false,[this, &ans, objTree]
        (int objectId){
            if (objTree->getObjectVisibility()[objectId] != ObjectTree::VisibilityState::FullyVisible) return true;

            for (int childObjectId : document->getObjectTree()->getChildren()[objectId]) {
                if (objTree->getObjectVisibility()[childObjectId] != ObjectTree::VisibilityState::FullyVisible) return true;
            }
            
            ans << objTree->getFullPathMap()[objectId];
            return false;
        }
        );

    }

    if (name == Name::BASENAME) {
        for (int i = 0; i < ans.size(); i++) 
            ans[i] = bu_path_basename(ans[i].toStdString().c_str(), NULL);
    }

    return ans;
}