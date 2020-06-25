//
// Created by Sadeep on 12-Jun.
//
#include "GeometryRenderer.h"

using namespace std;

GeometryRenderer::GeometryRenderer(Display *display) :display(display){
    setFGColor(defaultWireColor[0],defaultWireColor[1],defaultWireColor[2],1);
}

void GeometryRenderer::drawVList(BRLCAD::VectorList * vectorList) {

    mflag = 1;
    first = 1;

    glGetFloatv(GL_POINT_SIZE, &originalPointSize);
    glGetFloatv(GL_LINE_WIDTH, &originalLineWidth);

    vectorList->Iterate(*this);

    if (first == 0) glEnd();
    if (dmLight && dmTransparency)  glDisable(GL_BLEND);
    glPointSize(originalPointSize);
    glLineWidth(originalLineWidth);
}

bool GeometryRenderer::operator()(BRLCAD::VectorList::Element *element) {
    if (!element) return true;

    switch (element->Type()) {

        case BRLCAD::VectorList::Element::LineDraw:{
            auto *e = dynamic_cast<BRLCAD::VectorList::LineDraw *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::LineMove: {
            auto *e = dynamic_cast<BRLCAD::VectorList::LineMove *> (element);
            if (first == 0) glEnd();
            first = 0;

            if (dmLight && mflag) {
                mflag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, wireColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);

                if (dmTransparency) glDisable(GL_BLEND);
            }

            glBegin(GL_LINE_STRIP);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::ModelSpace: {
            if (first == 0) {
                glEnd();
                first = 1;
            }

            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            break;
        }
        case BRLCAD::VectorList::Element::DisplaySpace: {
            auto *e = dynamic_cast<BRLCAD::VectorList::DisplaySpace *> (element);
            glMatrixMode(GL_MODELVIEW);
            glGetDoublev(GL_MODELVIEW_MATRIX, m);

            MAT_TRANSPOSE(mt, m);
            MAT4X3PNT(tlate, mt, e->ReferencePoint().coordinates);

            class Display;
            glPushMatrix();
            glLoadIdentity();
            glTranslated(tlate[0], tlate[1], tlate[2]);
            /* 96 dpi = 3.78 pixel/mm hardcoded */
            glScaled(2. * 3.78 / display->getW(),
                     2. * 3.78 / display->getH(),
                     1.);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonStart:{
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonStart *> (element);
            if (dmLight && mflag) {
                mflag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

                switch (dmLight) {
                    case 1:
                        break;
                    case 2:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, diffuseColor);
                        break;
                    case 3:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, backDiffuseColorDark);
                        break;
                    default:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, backDiffuseColorLight);
                        break;
                }

                if (dmTransparency)
                    glEnable(GL_BLEND);
            }

            if (first == 0) glEnd();
            glBegin(GL_POLYGON);

            glNormal3dv(e->Normal().coordinates);

            first = 0;
            break;
        }
        case BRLCAD::VectorList::Element::TriangleStart: {
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleStart *> (element);
            if (dmLight && mflag) {
                mflag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

                switch (dmLight) {
                    case 1:
                        break;
                    case 2:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, diffuseColor);
                        break;
                    case 3:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, backDiffuseColorDark);
                        break;
                    default:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, backDiffuseColorLight);
                        break;
                }

                if (dmTransparency)
                    glEnable(GL_BLEND);
            }

            if (first) {
                glBegin(GL_TRIANGLES);
            }

            glNormal3dv(e->Normal().coordinates);

            first = 0;
            break;
        }
        case BRLCAD::VectorList::Element::PolygonMove:{
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonMove *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonDraw:{
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonDraw *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::TriangleMove:{
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleMove *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::TriangleDraw: {
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleDraw *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonEnd: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonEnd *> (element);
            glVertex3dv(e->Point().coordinates);
            glEnd();
            first = 1;
            break;
        }
        case BRLCAD::VectorList::Element::TriangleEnd: {
            break;
        }
        case BRLCAD::VectorList::Element::PolygonVertexNormal: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonVertexNormal *> (element);
            glNormal3dv(e->Normal().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::TriangleVertexNormal: {
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleVertexNormal *> (element);
            glNormal3dv(e->Normal().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::PointDraw: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PointDraw *> (element);
            if (first == 0) glEnd();
            first = 0;
            glBegin(GL_POINTS);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::LineWidth: {
            auto *e = dynamic_cast<BRLCAD::VectorList::LineWidth *> (element);
            auto lineWidth = static_cast<float>(e->Width());
            if (lineWidth > 0.0) {
                glLineWidth(lineWidth);
            }
            break;
        }
        case BRLCAD::VectorList::Element::PointSize: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PointSize *> (element);
            auto pointSize = static_cast<float>(e->Size());
            if (pointSize > 0.0) {
                glPointSize(pointSize);
            }
            break;
        }
    }
    return true;
}


tree* GeometryRenderer::buildAndDrawSolids
        (
                db_tree_state*      tsp,
                const db_full_path* pathp,
                rt_db_internal*     ip,
                void*               clientData
        ) {
    tree*    ret   = TREE_NULL;

    BRLCAD::VectorList vectorList;
    auto * gr = static_cast<GeometryRenderer *>(clientData);

    if (ip->idb_meth->ft_plot != 0) {
        if (ip->idb_meth->ft_plot(vectorList.m_vlist, ip, tsp->ts_ttol, tsp->ts_tol, 0) == 0) {
            BU_GET(ret, tree);
            RT_TREE_INIT(ret);
            ret->tr_op = OP_NOP;
        }
    }

    GLuint dlist = glGenLists(1);
    glNewList(dlist, GL_COMPILE);
    gr->setFGColor(tsp->ts_mater.ma_color[0],tsp->ts_mater.ma_color[1],tsp->ts_mater.ma_color[2],1);
    gr->drawVList(&vectorList);
    glEndList();

    Solid solid{};
    solid.dlist = dlist;
    solid.color[0] = tsp->ts_mater.ma_color[0];
    solid.color[1] = tsp->ts_mater.ma_color[1];
    solid.color[2] = tsp->ts_mater.ma_color[2];
    gr->solids.push_back(solid);

    return ret;
}

void GeometryRenderer::refreshGeometry() {
    const char * Goliathc = "Goliath.c";

    db_tree_state initState;
    db_init_db_tree_state(&initState, display->getDatabase()->m_rtip->rti_dbip, display->getDatabase()->m_resp);
    initState.ts_ttol = &display->getDatabase()->m_rtip->rti_ttol;
    initState.ts_tol  = &display->getDatabase()->m_rtip->rti_tol;

    display->makeCurrent();
    solids.clear();

    db_walk_tree(display->getDatabase()->m_rtip->rti_dbip,
                 1,
                 &Goliathc,
                 1,
                 &initState,
                 0,
                 0,
                 GeometryRenderer::buildAndDrawSolids,
                 this);

}

void GeometryRenderer::setFGColor( float r, float g, float b, float transparency){

    wireColor[0] = r;
    wireColor[1] = g;
    wireColor[2] = b;
    wireColor[3] = transparency;

    diffuseColor[0] = wireColor[0] * 0.6f;
    diffuseColor[1] = wireColor[1] * 0.6f;
    diffuseColor[2] = wireColor[2] * 0.6f;
    diffuseColor[3] = wireColor[3];

    ambientColor[0] = wireColor[0] * 0.2f;
    ambientColor[1] = wireColor[1] * 0.2f;
    ambientColor[2] = wireColor[2] * 0.2f;
    ambientColor[3] = wireColor[3];

    specularColor[0] = ambientColor[0];
    specularColor[1] = ambientColor[1];
    specularColor[2] = ambientColor[2];
    specularColor[3] = ambientColor[3];

    backDiffuseColorDark[0] = wireColor[0] * 0.3f;
    backDiffuseColorDark[1] = wireColor[1] * 0.3f;
    backDiffuseColorDark[2] = wireColor[2] * 0.3f;
    backDiffuseColorDark[3] = wireColor[3];

    backDiffuseColorLight[0] = wireColor[0] * 0.9f;
    backDiffuseColorLight[1] = wireColor[1] * 0.9f;
    backDiffuseColorLight[2] = wireColor[2] * 0.9f;
    backDiffuseColorLight[3] = wireColor[3];

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor);
}

void GeometryRenderer::render() {
    GLboolean currentLightingStatus;
    glGetBooleanv(GL_LIGHTING,&currentLightingStatus);
    glEnable(GL_LIGHTING);
    for(auto i:solids){
        drawDList(i.dlist);
    }
    if(!currentLightingStatus)glDisable(GL_LIGHTING);
}

void GeometryRenderer::drawDList(unsigned int list)
{
    glCallList((GLuint)list);
}

void GeometryRenderer::initialize() {
    if (initialized){
        return;
    }
    initialized = true;
    refreshGeometry();
}
