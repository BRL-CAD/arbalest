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

#include <brlcad/Combination.h>
#include "GeometryRenderer.h"


GeometryRenderer::GeometryRenderer(DisplayManager* displayManager, Document* document) : displayManager(displayManager), document(document)
{
	
}
void GeometryRenderer::render() {
    if (document->getDatabase() == nullptr) return;
    displayManager->saveState();

    // If database has been updated we need to redraw.
    // Also QOpenGLWidget sometimes looses saved display list after UI changes (dock / undock etc).
    // Therefore we need to draw again if the dlists are not available
    if (databaseUpdated || (!solids.empty() && !displayManager->isDListValid(solids[0]))) {
        drawDatabase();
        databaseUpdated = false;
    }
    for (int i:solids) {
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


void GeometryRenderer::DatabaseWalker::operator()(BRLCAD::Object& object) {
    const BRLCAD::Combination* comb = dynamic_cast<const BRLCAD::Combination*>(&object);
    if (comb != 0) {
        if(comb->HasColor()) {
            colorInfo.red = comb->Red();
            colorInfo.green = comb->Green();
            colorInfo.blue = comb->Blue();
            colorInfo.hasColor = true;
        }
        ListTreeNode(comb->Tree());
    }
    else{
        geometryRenderer.drawSolid(path.c_str(), colorInfo);
    }
}

void GeometryRenderer::DatabaseWalker::ListTreeNode(const BRLCAD::Combination::ConstTreeNode& node){
    switch (node.Operation()) {
        case BRLCAD::Combination::ConstTreeNode::Union:
        case BRLCAD::Combination::ConstTreeNode::Intersection:
        case BRLCAD::Combination::ConstTreeNode::Subtraction:
        case BRLCAD::Combination::ConstTreeNode::ExclusiveOr:
            ListTreeNode(node.LeftOperand());
            ListTreeNode(node.RightOperand());
            break;

        case BRLCAD::Combination::ConstTreeNode::Not:
            ListTreeNode(node.Operand());
            break;

        case BRLCAD::Combination::ConstTreeNode::Leaf:
            const char * leafName = node.Name();
            std::string leafPath = path + "/" + std::string(leafName);
            DatabaseWalker callback(database, geometryRenderer, leafPath, colorInfo);
            database->Get(leafName, callback);
    }
}

/*
 * Clears existing display lists, iterate through each solid and generates display lists by calling drawSolid on each
 */
void GeometryRenderer::drawDatabase() {
    for (int i: solids){
        displayManager->freeDLists(i,1);
    }
    solids.clear();

    BRLCAD::ConstDatabase::TopObjectIterator it = document->getDatabase()->FirstTopObject();
    while (it.Good()) {
        std::string objectName = std::string(it.Name());
        ColorInfo colorInfo{};
        colorInfo.hasColor = false;
        DatabaseWalker walker(document->getDatabase(), *this, objectName, colorInfo);
        document->getDatabase()->Get(it.Name(), walker);
        ++it;
    }

}


/*
 * Set the color and line attribute to suit a given solid, creates a display list, plots and draws solid's vlist into the display list.
 * The created display list is added to GeometryRenderer::solids
 */
void
GeometryRenderer::drawSolid(const char *name, GeometryRenderer::ColorInfo colorInfo) {
    BRLCAD::VectorList vectorList;
    document->getDatabase()->Plot(name,vectorList);


    GLuint dlist;
    dlist = displayManager->genDLists(1);
    displayManager->beginDList(dlist);  // begin display list --------------
    solids.push_back(dlist);

    if (colorInfo.hasColor) {
        displayManager->setFGColor(colorInfo.red, colorInfo.green, colorInfo.blue, 1);
    }
    else {
        displayManager->setFGColor(defaultWireColor[0], defaultWireColor[1], defaultWireColor[2], 1);
    }

    //displayManager->setLineStyle(tsp->ts_sofar & (TS_SOFAR_MINUS | TS_SOFAR_INTER));
    displayManager->drawVList(&vectorList);
    displayManager->endDList();     // end display list --------------
}

