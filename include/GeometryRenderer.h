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
#include <brlcad/Combination.h>

class GeometryRenderer:public Renderer {
public:
    GeometryRenderer(DisplayManager* displayManager, Document* document);

    // This method should be called database is changed (i.e. after changing this->database) or updated
    void onDatabaseUpdated();

    // this is called by Display to render a single frame
    void render() override;

    // used to represent color as traversed through database
    struct ColorInfo{
        float red,green,blue;
        bool hasColor;
    };

    // draw object and add its display list to solids
    void drawSolid(const char *name, GeometryRenderer::ColorInfo colorInfo);

    // traversing through the database
    class DatabaseWalker: public BRLCAD::MemoryDatabase::ObjectCallback{
    public:
        DatabaseWalker(BRLCAD::MemoryDatabase*database, GeometryRenderer &geometryRenderer, std::string &path,
                       ColorInfo colorInfo)
                : colorInfo(colorInfo),
                  database(database),
                  geometryRenderer(geometryRenderer), path(path) {}
        void operator()(BRLCAD::Object& object) override;
        void ListTreeNode(const BRLCAD::Combination::ConstTreeNode& node);

    private:
        ColorInfo colorInfo;
        BRLCAD::MemoryDatabase * database;
        GeometryRenderer & geometryRenderer;
        std::string& path;
    };

private:
    DisplayManager* displayManager;
    Document* document;
    float defaultWireColor[3] = {1.0,.1,.4};
    bool databaseUpdated = false;

    void drawDatabase();
    std::vector<int> solids; // contains the display list of each solid
};


#endif //BRLCAD_GEOMETRYRENDERER_H
