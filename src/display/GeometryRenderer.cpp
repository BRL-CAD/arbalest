//
// Created by Sadeep on 12-Jun.
//
#include "GeometryRenderer.h"


#define DM_SOLID_LINE 0
#define DM_DASHED_LINE 1

GeometryRenderer::GeometryRenderer(Display *display) : display(display) {
    setFGColor(defaultWireColor[0], defaultWireColor[1], defaultWireColor[2], 1);
}

void GeometryRenderer::drawVList(BRLCAD::VectorList *vectorList) {

    mflag = 1;
    first = 1;

    vectorList->Iterate(*this);

    if (first == 0) glEnd();
    if (dmLight && dmTransparency) glDisable(GL_BLEND);
}

bool GeometryRenderer::operator()(BRLCAD::VectorList::Element *element) {
    if (!element) return true;

    switch (element->Type()) {

        case BRLCAD::VectorList::Element::LineDraw: {
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
        case BRLCAD::VectorList::Element::PolygonStart: {
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
        case BRLCAD::VectorList::Element::PolygonMove: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonMove *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonDraw: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonDraw *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::TriangleMove: {
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


tree *GeometryRenderer::drawSolid
        (
                db_tree_state *tsp,
                const db_full_path *pathp,
                rt_db_internal *ip,
                void *clientData
        ) {
    tree *ret = TREE_NULL;

    BRLCAD::VectorList vectorList;
    auto *gr = static_cast<GeometryRenderer *>(clientData);

    if (ip->idb_meth->ft_plot != 0) {
        if (ip->idb_meth->ft_plot(vectorList.m_vlist, ip, tsp->ts_ttol, tsp->ts_tol, 0) == 0) {
            BU_GET(ret, tree);
            RT_TREE_INIT(ret);
            ret->tr_op = OP_NOP;
        }
    }

    GLuint dlist = glGenLists(1);
    glNewList(dlist, GL_COMPILE);

    if (tsp->ts_mater.ma_color_valid) {
        gr->setFGColor(tsp->ts_mater.ma_color[0], tsp->ts_mater.ma_color[1], tsp->ts_mater.ma_color[2], 1);
    }
    else {
        gr->setFGColor(gr->defaultWireColor[0], gr->defaultWireColor[1], gr->defaultWireColor[2], 1);
    }

    gr->setLineAttr(-1,tsp->ts_sofar & (TS_SOFAR_MINUS|TS_SOFAR_INTER));
    gr->drawVList(&vectorList);
    glEndList();

    Solid solid{};
    solid.dlist = dlist;
    gr->solids.push_back(solid);

    return ret;
}

void GeometryRenderer::refreshGeometry() {

    db_tree_state initState;
    db_init_db_tree_state(&initState, display->getDatabase()->m_rtip->rti_dbip, display->getDatabase()->m_resp);
    initState.ts_ttol = &display->getDatabase()->m_rtip->rti_ttol;
    initState.ts_tol = &display->getDatabase()->m_rtip->rti_tol;

    display->makeCurrent();
    solids.clear();

    glLineStipple(1, 0xCF33);
    GLfloat currentLineWidth;
    GLboolean currentLightingStatus;
    GLboolean currentLineStippleStatus;
    glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
    glGetBooleanv(GL_LIGHTING, &currentLightingStatus);
    glGetBooleanv(GL_LINE_STIPPLE, &currentLineStippleStatus);
    glEnable(GL_LIGHTING);

    BRLCAD::ConstDatabase::TopObjectIterator it = display->getDatabase()->FirstTopObject();
    while (it.Good()) {
        const char *topObjectName = it.Name();
        db_walk_tree(display->getDatabase()->m_rtip->rti_dbip, 1, &topObjectName, 1, &initState, 0, 0,
                     GeometryRenderer::drawSolid, this);

        ++it;
    }

    glLineWidth(currentLineWidth);
    if (currentLightingStatus) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);
    if (currentLineStippleStatus) glEnable(GL_LINE_STIPPLE);
    else glDisable(GL_LINE_STIPPLE);
}

void GeometryRenderer::setFGColor(float r, float g, float b, float transparency) {

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

void GeometryRenderer::setLineAttr(int width, int style)
{
    if (width>0)
        glLineWidth((GLfloat) width);

    if (style == DM_DASHED_LINE)
        glEnable(GL_LINE_STIPPLE);
    else
        glDisable(GL_LINE_STIPPLE);

}

void GeometryRenderer::render() {
    if (!initialized) {
        initialized = true;
        refreshGeometry();
    }

    GLfloat currentLineWidth;
    GLboolean currentLightingStatus;
    GLboolean currentLineStippleStatus;
    glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
    glGetBooleanv(GL_LIGHTING, &currentLightingStatus);
    glGetBooleanv(GL_LINE_STIPPLE, &currentLineStippleStatus);
    glEnable(GL_LIGHTING);

    for (auto i:solids) {
        drawDList(i.dlist);
    }

    glLineWidth(currentLineWidth);
    if (currentLightingStatus) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);
    if (currentLineStippleStatus) glEnable(GL_LINE_STIPPLE);
    else glDisable(GL_LINE_STIPPLE);
}

void GeometryRenderer::drawDList(unsigned int list) {
    glCallList((GLuint) list);
}

