//
// Created by Sadeep on 12-Jun.
//

#ifndef RT3_GEOMETRYRENDERER_H
#define RT3_GEOMETRYRENDERER_H
#ifdef _WIN32
#include <Windows.h>
#endif


#include <brlcad/MemoryDatabase.h>
#include <brlcad/cicommon.h>
#include <brlcad/Object.h>
#include <gl/GL.h>
#include <brlcad/bn/vlist.h>
#include <brlcad/rt/tree.h>
#include <brlcad/rt/rt_instance.h>
#include <brlcad/rt/functab.h>
#include <brlcad/rt/db_internal.h>
#include "vmath.h"
#include "Solid.h"
#include "Display.h"


struct rt_i;

class GeometryRenderer : public BRLCAD::VectorList::ElementCallback {
public:
    explicit GeometryRenderer(Display *display);
    void drawVList(BRLCAD::VectorList * vectorList) ;
    bool operator()(BRLCAD::VectorList::Element* element) override ;
    void refreshGeometry();
    void setFGColor(float r, float g, float b, float transparency);

    void render();

    void initialize();

private:
    Display *display;
    int dm_light = 1;
    bool dm_transparency = 0;
    int	first;
    int mflag = 1;
    GLfloat originalPointSize, originalLineWidth;
    GLdouble m[16];
    GLdouble mt[16];
    GLdouble tlate[3];
    const  float black[4] = {0.0, 0.0, 0.0, 0.0};
    const  unsigned char defaultWireColor[3] = {0,250,0};
    float wireColor[4] = {.9,.1,.1,1};
    float diffuseColor[4];
    float  ambientColor[4];
    float  specularColor[4];
    float  backDiffuseColorDark[4];
    float backDiffuseColorLight[4];

    std::vector<Solid> solids;

    static tree *buildAndDrawSolids(db_tree_state *tsp, const db_full_path *pathp, rt_db_internal *ip, void *clientData);

    void drawDList(unsigned int list);

    bool initialized = false;

};


#endif //RT3_GEOMETRYRENDERER_H
