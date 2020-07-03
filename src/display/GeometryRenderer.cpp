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

#include "common.h"
#include "GeometryRenderer.h"

GeometryRenderer::GeometryRenderer(DisplayManager *displayManager) : displayManager(displayManager) {}

void GeometryRenderer::render() {
    if (database == nullptr) return;
    displayManager->saveState();

    // If database has been updated we need to redraw.
    // Also QOpenGLWidget sometimes looses saved display list after UI changes (dock / undock etc).
    // Therefore we need to draw again if the dlists are not available
    if (databaseUpdated || (!solids.empty() && !displayManager->isDListValid(solids[0]))) {
        drawDatabase();
        databaseUpdated = false;
    }
    for (auto i:solids) {
        displayManager->drawDList(i);
    }
    displayManager->restoreState();
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
    for (auto i: solids){
        displayManager->freeDLists(i,1);
    }
    solids.clear();

    BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();
    while (it.Good()) {
        const char * objectName = it.Name();
        database->WalkTree(&objectName,drawSolid,this);
        ++it;
    }
}


/*
 * Set the color and line attribute to suit a given solid, creates a display list, plots and draws solid's vlist into the display list.
 * The created display list is added to GeometryRenderer::solids
 */
void GeometryRenderer::drawSolid(BRLCAD::ConstDatabase::TreeLeaf *treeLeaf ,void *clientData) {
    auto *geometryRenderer = static_cast<GeometryRenderer *>(clientData);
    auto *displayManager = geometryRenderer->displayManager;

    BRLCAD::VectorList vectorList;
    treeLeaf->Plot(vectorList);


    GLuint dlist;
    dlist = displayManager->genDLists(1);
    displayManager->beginDList(dlist);  // begin display list --------------
    geometryRenderer->solids.push_back(dlist);

    if (treeLeaf->IsMaterialColorValid()) {
        auto color = treeLeaf->MaterialColor();
        displayManager->setFGColor(color[0], color[1], color[2], 1);
    }
    else {
        displayManager->setFGColor(geometryRenderer->defaultWireColor[0], geometryRenderer->defaultWireColor[1],
                geometryRenderer->defaultWireColor[2], 1);
    }

    //displayManager->setLineStyle(tsp->ts_sofar & (TS_SOFAR_MINUS | TS_SOFAR_INTER));
    displayManager->drawVList(&vectorList);
    displayManager->endDList();     // end display list --------------
}

void GeometryRenderer::setDatabase(BRLCAD::MemoryDatabase *database) {
    this->database = database;
    onDatabaseUpdated();
}
