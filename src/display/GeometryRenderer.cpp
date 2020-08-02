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

}

void GeometryRenderer::render() {
    if (document->getDatabase() == nullptr) return;
    document->getDisplay()->getDisplayManager()->saveState();

    if (!document->getObjectTree()->getObjectsToBeDrawnIds().isEmpty()) {
        for (int objectId : document->getObjectTree()->getObjectsToBeDrawnIds())
        {
            // if object was already drawn earlier, need to remove the previous display list
            if (document->getObjectTree()->getObjectIdDisplayListIdMap().contains(objectId))
            {
                const int previousDisplayListId = document->getObjectTree()->getObjectIdDisplayListIdMap()[objectId];
                document->getDisplay()->getDisplayManager()->freeDLists(previousDisplayListId, 1); // free ot from openGL
                // no need to remove from objectIdDisplayListIdMap or visibleDisplayListIds since they'll be dealt with later
            }

            const QString objectFullPath = document->getObjectTree()->getFullPathMap()[objectId];
            const ColorInfo objectColor = document->getObjectTree()->getColorMap()[objectId];
            const int  displayListId = drawSolid(objectFullPath.toUtf8(), objectColor);
            document->getObjectTree()->getObjectIdDisplayListIdMap()[objectId] = displayListId;
            static int a = 0;
        }

        document->getObjectTree()->rebuildVisibleDisplayListIds();
        document->getObjectTree()->getObjectsToBeDrawnIds().clear();
    }

    for (int i : document->getObjectTree()->getVisibleDisplayListIds()) {
        document->getDisplay()->getDisplayManager()->drawDList(i);
    }
    document->getDisplay()->getDisplayManager()->restoreState();
}


/*
 * Set the color and line attribute to suit a given solid, creates a display list, plots and draws solid's vlist into the display list.
 * The created display list is added to GeometryRenderer::solids
 */
int GeometryRenderer::drawSolid(const char* name, ColorInfo colorInfo) {
    BRLCAD::VectorList vectorList;
    document->getDatabase()->Plot(name, vectorList);

    const int displayListId = document->getDisplay()->getDisplayManager()->genDLists(1);
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

    return displayListId;
}

