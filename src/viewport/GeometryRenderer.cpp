/*            G E O M E T R Y R E N D E R E R . C P P
 * BRL-CAD
 *
 * Copyright (c) 2020 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
 /** @file GeometryRenderer.cpp */

#include "GeometryRenderer.h"


GeometryRenderer::GeometryRenderer(Document* document) : document(document)
{
    refreshForVisibilityAndSolidChanges();
}

void GeometryRenderer::render() {
    document->getViewport()->getViewportManager()->saveState();
    if (!objectsToBeViewportedIds.empty()) {
        for (size_t objectId : objectsToBeViewportedIds) {
            if (!objectIdViewportListIdMap.contains(objectId))
                drawSolid(objectId);
            visibleViewportListIds.append(objectIdViewportListIdMap[objectId]);
        }
        objectsToBeViewportedIds.clear();
    }

    for (size_t displayListId : visibleViewportListIds)
        document->getViewport()->getViewportManager()->drawDList(displayListId);

    document->getViewport()->getViewportManager()->drawSuffix();
    document->getViewport()->getViewportManager()->restoreState();
}


void GeometryRenderer::drawSolid(size_t objectId) {
    ObjectTreeItem *item = document->getObjectTree()->getItems()[objectId];
    const ColorInfo colorInfo = item->getColorInfo();
    const QString objectFullPath = item->getPath();
    BRLCAD::VectorList vectorList;
    document->getDatabase()->Plot(objectFullPath.toUtf8(), vectorList);

    clearSolidIfAvailable(objectId);

    const size_t displayListId = document->getViewport()->getViewportManager()->genDLists(1);
    document->getViewport()->getViewportManager()->beginDList(displayListId);  // begin display list --------------

    if (colorInfo.hasColor) {
        document->getViewport()->getViewportManager()->setFGColor(colorInfo.red, colorInfo.green, colorInfo.blue, 1);
    }
    else {
        document->getViewport()->getViewportManager()->setFGColor(defaultWireColor[0], defaultWireColor[1], defaultWireColor[2], 1);
    }

    //displayManager->setLineStyle(tsp->ts_sofar & (TS_SOFAR_MINUS | TS_SOFAR_INTER));
    document->getViewport()->getViewportManager()->drawVList(&vectorList);
    document->getViewport()->getViewportManager()->endDList();     // end display list --------------

    objectIdViewportListIdMap[objectId] = displayListId;
}



void GeometryRenderer::refreshForVisibilityAndSolidChanges() {
    visibleViewportListIds.clear();
    document->getObjectTree()->traverseSubTree(0, false, [this](size_t objectId)
        {
            ObjectTreeItem *item = document->getObjectTree()->getItems()[objectId];
            if (item->getVisibilityState() == ObjectTreeItem::Invisible) return false;
            if (!item->isDrawable()) return true;
            objectsToBeViewportedIds.append(objectId);
            return true;
        }
    );
}

void GeometryRenderer::clearSolidIfAvailable(size_t objectId) {
    if (objectIdViewportListIdMap.contains(objectId)){
        document->getViewport()->getViewportManager()->freeDLists(objectIdViewportListIdMap[objectId], 1);
        objectIdViewportListIdMap.remove(objectId);
    }
}

void GeometryRenderer::clearObject(size_t objectId) {
    document->getObjectTree()->traverseSubTree(objectId, true, [this](size_t objectId){
        clearSolidIfAvailable(objectId);
        return true;
    });
}


