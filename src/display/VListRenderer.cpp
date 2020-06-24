//
// Created by Sadeep on 12-Jun.
//
#ifdef _WIN32

#include <Windows.h>

#endif

#include <gl/GL.h>
#include "VListRenderer.h"
#include "vmath.h"
#include <bn.h>
#include "raytrace.h"

using namespace std;

void VListRenderer::render(bn_vlist *vp, int w, int h) {

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

    struct bn_vlist *tvp;
    int first;
    int mflag = 1;
    float black[4] = {0.0, 0.0, 0.0, 0.0};
    GLfloat originalPointSize, originalLineWidth;
    GLdouble m[16];
    GLdouble mt[16];
    GLdouble tlate[3];

    glGetFloatv(GL_POINT_SIZE, &originalPointSize);
    glGetFloatv(GL_LINE_WIDTH, &originalLineWidth);

    /* Viewing region is from -1.0 to +1.0 */
    first = 1;

    for (BU_LIST_FOR(tvp, bn_vlist, &vp->l)) {
        int i;
        int nused = tvp->nused;
        int *cmd = tvp->cmd;
        point_t *pt = tvp->pt;
        GLdouble glpt[3];

        for (i = 0; i < nused; i++, cmd++, pt++) {
            VMOVE(glpt, *pt);

            switch (*cmd) {
                case BN_VLIST_LINE_MOVE:
                    /* Move, start line */
                    if (first == 0)
                        glEnd();
                    first = 0;

                    if (dm_light && mflag) {
                        mflag = 0;
                        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, wireColor);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);

                        if (dm_transparency)
                            glDisable(GL_BLEND);
                    }

                    glBegin(GL_LINE_STRIP);
                    glVertex3dv(glpt);
                    break;
                case BN_VLIST_MODEL_MAT:
                    if (first == 0) {
                        glEnd();
                        first = 1;
                    }

                    glMatrixMode(GL_MODELVIEW);
                    glPopMatrix();
                    break;
                case BN_VLIST_DISPLAY_MAT:
                    glMatrixMode(GL_MODELVIEW);
                    glGetDoublev(GL_MODELVIEW_MATRIX, m);

                    MAT_TRANSPOSE(mt, m);
                    MAT4X3PNT(tlate, mt, glpt);

                    glPushMatrix();
                    glLoadIdentity();
                    glTranslated(tlate[0], tlate[1], tlate[2]);
                    /* 96 dpi = 3.78 pixel/mm hardcoded */
                    glScaled(2. * 3.78 / w,
                             2. * 3.78 / h,
                             1.);
                    break;
                case BN_VLIST_POLY_START:
                case BN_VLIST_TRI_START:
                    /* Start poly marker & normal */

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

                    if (*cmd == BN_VLIST_POLY_START) {
                        if (first == 0)
                            glEnd();

                        glBegin(GL_POLYGON);
                    } else if (first)
                        glBegin(GL_TRIANGLES);

                    /* Set surface normal (vl_pnt points outward) */
                    glNormal3dv(glpt);

                    first = 0;

                    break;
                case BN_VLIST_LINE_DRAW:
                case BN_VLIST_POLY_MOVE:
                case BN_VLIST_POLY_DRAW:
                case BN_VLIST_TRI_MOVE:
                case BN_VLIST_TRI_DRAW:
                    glVertex3dv(glpt);

                    break;
                case BN_VLIST_POLY_END:
                    /* Draw, End Polygon */
                    glVertex3dv(glpt);
                    glEnd();
                    first = 1;
                    break;
                case BN_VLIST_TRI_END:
                    break;
                case BN_VLIST_POLY_VERTNORM:
                case BN_VLIST_TRI_VERTNORM:
                    /* Set per-vertex normal.  Given before vert. */
                    glNormal3dv(glpt);
                    break;
                case BN_VLIST_POINT_DRAW:
                    if (first == 0)
                        glEnd();
                    first = 0;
                    glBegin(GL_POINTS);
                    glVertex3dv(glpt);
                    break;
                case BN_VLIST_LINE_WIDTH: {
                    GLfloat lineWidth = (GLfloat) (*pt)[0];
                    if (lineWidth > 0.0) {
                        glLineWidth(lineWidth);
                    }
                    break;
                }
                case BN_VLIST_POINT_SIZE: {
                    GLfloat pointSize = (GLfloat) (*pt)[0];
                    if (pointSize > 0.0) {
                        glPointSize(pointSize);
                    }
                    break;
                }
            }
        }
    }

    if (first == 0)
        glEnd();

    if (dm_light && dm_transparency)
        glDisable(GL_BLEND);

    glPointSize(originalPointSize);
    glLineWidth(originalLineWidth);
    return;
}