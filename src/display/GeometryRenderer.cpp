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
    document->getDisplay()->getDisplayManager()->saveState();
    if (!objectsToBeDisplayedIds.empty()) {
        for (int objectId : objectsToBeDisplayedIds) {
            if (!objectIdDisplayListIdMap.contains(objectId)) {
                drawSolid(objectId);
            }
            visibleDisplayListIds.append(objectIdDisplayListIdMap[objectId]);
        }
        objectsToBeDisplayedIds.clear();
    }

    for (int displayListId : visibleDisplayListIds) {
        document->getDisplay()->getDisplayManager()->drawDList(displayListId);
    }
    document->getDisplay()->getDisplayManager()->drawSuffix();
    document->getDisplay()->getDisplayManager()->restoreState();
}


void GeometryRenderer::drawSolid(int objectId) {
    const ColorInfo colorInfo = document->getObjectTree()->getColorMap()[objectId];
    const QString objectFullPath = document->getObjectTree()->getFullPathMap()[objectId];
    BRLCAD::VectorList vectorList;
    document->getDatabase()->Plot(objectFullPath.toUtf8(), vectorList);

    clearSolidIfAvailable(objectId);

    const unsigned int displayListId = document->getDisplay()->getDisplayManager()->genDLists(1);
    document->getDisplay()->getDisplayManager()->beginDList(displayListId);  // begin display list --------------

    if (colorInfo.hasColor) {
        document->getDisplay()->getDisplayManager()->setFGColor(colorInfo.red, colorInfo.green, colorInfo.blue, 1);
    }
    else {
        document->getDisplay()->getDisplayManager()->setFGColor(defaultWireColor[0], defaultWireColor[1], defaultWireColor[2], 1);
    }

    //displayManager->setLineStyle(tsp->ts_sofar & (TS_SOFAR_MINUS | TS_SOFAR_INTER));
    document->getDisplay()->getDisplayManager()->drawVList(&vectorList);
    document->getDisplay()->getDisplayManager()->endDList();     // end display list --------------

    objectIdDisplayListIdMap[objectId] = displayListId;
}



void GeometryRenderer::refreshForVisibilityAndSolidChanges() {
    visibleDisplayListIds.clear();
    document->getObjectTree()->traverseSubTree(0, false,[this]
        (int objectId)
        {
            if (document->getObjectTree()->getObjectVisibility()[objectId] == ObjectTree::Invisible) return false;
            if (!document->getObjectTree()->getDrawableObjectIds().contains(objectId)) return true;
            objectsToBeDisplayedIds.append(objectId);
            return true;
        }
    );
}

void GeometryRenderer::clearSolidIfAvailable(int objectId) {
    if (objectIdDisplayListIdMap.contains(objectId)){
        document->getDisplay()->getDisplayManager()->freeDLists(objectIdDisplayListIdMap[objectId], 1);
        objectIdDisplayListIdMap.remove(objectId);
    }
}

void GeometryRenderer::clearObject(int objectId) {
    document->getObjectTree()->traverseSubTree(objectId, true, [this](int objectId){
        clearSolidIfAvailable(objectId);
        return true;
    });
}


