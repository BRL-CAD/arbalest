/*              G E O M E T R Y R E N D E R E R . H
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
/** @file GeometryRenderer.h */

#ifndef BRLCAD_GEOMETRYRENDERER_H
#define BRLCAD_GEOMETRYRENDERER_H

#include "DisplayManager.h"
#include "Renderer.h"

class GeometryRenderer:public Renderer {
public:

    explicit GeometryRenderer(Document* document);

    // this is called by Display to render a single frame
    void render() override;

    void refreshForVisibilityAndSolidChanges();

private:
    Document* document;
    float defaultWireColor[3] = {1.0,.1,.4};


    void drawSolid(int objectId);
    void clearSolidIfAvailable(int objectId);
    void clearObject(int objectId);

    // Contains generated display list alone with corresponding objectId. objectId is the key. displayListId is value.
    QHash<int, int>             objectIdDisplayListIdMap;

    QVector<int> visibleDisplayListIds;
    QVector<int> objectsToBeDisplayedIds;
};


#endif //BRLCAD_GEOMETRYRENDERER_H
