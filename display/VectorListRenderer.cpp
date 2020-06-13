//
// Created by Sadeep on 12-Jun.
//
#ifdef _WIN32
#include <windows.h>
#endif

#include <gl/GL.h>
#include <Display.h>
#include <brlcad/bn/vlist.h>
#include "VectorListRenderer.h"
#include "vmath.h"
#include <iostream>
using namespace std;

int dm_light = 0;			/**< @brief !0 means lighting on */
bool dm_transparency = false;		/**< @brief !0 means transparency on */



int	first;
int mflag = 1;
GLfloat originalPointSize, originalLineWidth;
GLdouble m[16];
GLdouble mt[16];
GLdouble tlate[3];
//GLdouble glpt[3];

float black[4] = {0.0, 0.0, 0.0, 0.0};

float wireColor[4] = {.9,.5,.8,1};
float diffuseColor[4];
float  ambientColor[4];
float  specularColor[4];
float  backDiffuseColorDark[4];
float backDiffuseColorLight[4];


Display *activeDisplay;


int lol;
void VectorListRenderer::render(BRLCAD::VectorList *vectorList, Display * display) {

    activeDisplay = display;
    //display->makeCurrent();

    mflag = 1;
    memset(black, 0.0f, sizeof(black));
    glGetFloatv(GL_POINT_SIZE, &originalPointSize);
    glGetFloatv(GL_LINE_WIDTH, &originalLineWidth);
    first = 1;


    diffuseColor[0] = wireColor[0] * 0.6;
    diffuseColor[1] = wireColor[1] * 0.6;
    diffuseColor[2] = wireColor[2] * 0.6;
    diffuseColor[3] = wireColor[3];

    ambientColor[0] = wireColor[0] * 0.2;
    ambientColor[1] = wireColor[1] * 0.2;
    ambientColor[2] = wireColor[2] * 0.2;
    ambientColor[3] = wireColor[3];

    specularColor[0] = ambientColor[0];
    specularColor[1] = ambientColor[1];
    specularColor[2] = ambientColor[2];
    specularColor[3] = ambientColor[3];

    backDiffuseColorDark[0] = wireColor[0] * 0.9;
    backDiffuseColorDark[1] = wireColor[1] * 0.9;
    backDiffuseColorDark[2] = wireColor[2] * 0.9;
    backDiffuseColorDark[3] = wireColor[3];

    Callback elementOpenGL;

    lol = 0;
    vectorList->Iterate(elementOpenGL);
    //std::cout<<lol<<std::endl;

    bn_vlist* chunk;
    int l = 0;
    for (BU_LIST_FOR(chunk, bn_vlist, vectorList->m_vlist))
        for (size_t i = 0; i < chunk->nused; ++i)
            //std::cout<<(chunk->nused)<<std::endl;
            l++;
    //std::cout<<l<<std::endl;

}



bool VectorListRenderer::Callback::operator()(BRLCAD::VectorList::Element *element) {
    //if (!element) return true;
    assert(element);
    lol++;
    //return true;
    std::cout<<element->Type()<<std::endl;
    int loli = lol;
    int k = element->Type();
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

            if (dm_light && mflag) {
                mflag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, wireColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);

                if (dm_transparency) glDisable(GL_BLEND);
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

            glPushMatrix();
            glLoadIdentity();
            glTranslated(tlate[0], tlate[1], tlate[2]);
            /* 96 dpi = 3.78 pixel/mm hardcoded */
            glScaled(2. * 3.78 / activeDisplay->getW(),
                     2. * 3.78 / activeDisplay->getH(),
                     1.);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonStart:{
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonStart *> (element);
            if (dm_light && mflag) {
                mflag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

                switch (dm_light) {
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

                if (dm_transparency)
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
            if (dm_light && mflag) {
                mflag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

                switch (dm_light) {
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

                if (dm_transparency)
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
