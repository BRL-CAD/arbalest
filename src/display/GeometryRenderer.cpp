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

    // If database has been updated we need to redraw.
    // Also QOpenGLWidget sometimes looses saved display list after UI changes (dock / undock etc).
    // Therefore we need to draw again if the dlists are not available
    if (databaseUpdated || 
        (!document->getObjectTree()->getActiveDisplayListIds().empty() // (can't check for first element if empty)
            && 
            !document->getDisplay()->getDisplayManager()->isDListValid(document->getObjectTree()->getActiveDisplayListIds()[0]))) {
        drawDatabase();
        databaseUpdated = false;
    }
    for (int i:document->getObjectTree()->getActiveDisplayListIds()) {
        document->getDisplay()->getDisplayManager()->drawDList(i);
    }
    document->getDisplay()->getDisplayManager()->restoreState();
}

/*
 * This method should be called database is changed (i.e. after changing this->database) or updated
 */
void GeometryRenderer::onDatabaseUpdated() {
    databaseUpdated = true;
}

/*
 * Clears existing display lists, iterate through each solid and generates display lists by calling drawSolid on each
 */
void GeometryRenderer::drawDatabase() {
    for (int i: document->getObjectTree()->getActiveDisplayListIds()){
        document->getDisplay()->getDisplayManager()->freeDLists(i,1);
    }
    document->getObjectTree()->getActiveDisplayListIds().clear();

    for( int solidObjectId: document->getObjectTree()->getSolidObjectIds())
    {
        int  dlist = drawSolid(document->getObjectTree()->getFullNameMap()[solidObjectId].toUtf8(),
									{.2f, .2f, .2f, true});
        document->getObjectTree()->getActiveDisplayListIds().push_back(dlist);
    }
}


/*
 * Set the color and line attribute to suit a given solid, creates a display list, plots and draws solid's vlist into the display list.
 * The created display list is added to GeometryRenderer::solids
 */
int GeometryRenderer::drawSolid(const char *name, GeometryRenderer::ColorInfo colorInfo) {
    BRLCAD::VectorList vectorList;
    document->getDatabase()->Plot(name,vectorList);

    int dlist = document->getDisplay()->getDisplayManager()->genDLists(1);
    document->getDisplay()->getDisplayManager()->beginDList(dlist);  // begin display list --------------

    if (colorInfo.hasColor) {
        document->getDisplay()->getDisplayManager()->setFGColor(colorInfo.red, colorInfo.green, colorInfo.blue, 1);
    }
    else {
        document->getDisplay()->getDisplayManager()->setFGColor(defaultWireColor[0], defaultWireColor[1], defaultWireColor[2], 1);
    }

    //displayManager->setLineStyle(tsp->ts_sofar & (TS_SOFAR_MINUS | TS_SOFAR_INTER));
    document->getDisplay()->getDisplayManager()->drawVList(&vectorList);
    document->getDisplay()->getDisplayManager()->endDList();     // end display list --------------

    return dlist;
}

