//
// Created by sadeep on 6/2/19.
//

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <vmath.h>
#include <bn.h>
#include <thread>
#include "GraphicsViewOpenGL.h"
using namespace std;





GraphicsViewOpenGL::GraphicsViewOpenGL() : QGLWidget() {

}


void GraphicsViewOpenGL::initializeGL() {

}


void GraphicsViewOpenGL::resizeGL(int w, int h) {
    /*

    int WIDTH = 500;
    int HEIGHT = 1024;
    int width = w, height = h;
    const float ar_origin = (float) WIDTH / (float) HEIGHT;
    const float ar_new = (float) width / (float) height;

    float scale_w = (float) width / (float) WIDTH;
    float scale_h = (float) height / (float) HEIGHT;
    if (ar_new > ar_origin) {
        scale_w = scale_h;
    } else {
        scale_h = scale_w;
    }

    float margin_x = (width - WIDTH * scale_w) / 2;
    float margin_y = (height - HEIGHT * scale_h) / 2;

    glViewport(margin_x, margin_y, WIDTH * scale_w, HEIGHT * scale_h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH / ar_origin, 0, HEIGHT / ar_origin, 0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;*/


    GLint mm;

    static double xlim_view = 1.0;    /* args for glOrtho*/
    static double ylim_view = 1.0;

    //dmp->dm_aspect = (fastf_t) dmp->dm_width / (fastf_t) dmp->dm_height;



    glViewport(0, 0, w, h);

    glClearColor(0.0,0.0,0.0, 0.0);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glGetIntegerv(GL_MATRIX_MODE, &mm);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-w, w, -h, h, 0, 100000);
    //glFrustum(-w, w, -h, h, 100, 2000);
    gluPerspective(30, 1.0, 20, 10000);


    glTranslatef(-59*100*0,0,0);


    if(vp==0)return;


}

int h=0;


void GraphicsViewOpenGL::ogl_drawVList() {

    if(vp==0)return;
    //
    //printf("%d\n", onn);
    struct bn_vlist *tvp;
    register int first;
    register int mflag = 1;
    static float black[4] = {0.0, 0.0, 0.0, 0.0};
    GLfloat originalPointSize, originalLineWidth;
    GLfloat m[16];

    glGetFloatv(GL_POINT_SIZE, &originalPointSize);
    glGetFloatv(GL_LINE_WIDTH, &originalLineWidth);

    //if (dmp->dm_debugLevel == 1)

    /* Viewing region is from -1.0 to +1.0 */
    first = 1;
    int onn = 0;
    for (BU_LIST_FOR(tvp, bn_vlist, &vp->l)) {
        onn++;
        //if(onn==1)continue;
        int i;
        int nused = tvp->nused;
        int *cmd = tvp->cmd;
        point_t *pt = tvp->pt;
        for (i = 0; i < nused; i++, cmd++, pt++) {
            GLdouble dpt[3];
            VMOVE(dpt, *pt);
/*
	    if (dmp->dm_debugLevel > 2)
		bu_log(" %d (%g %g %g)\n", *cmd, V3ARGS(dpt));*/

            switch (*cmd) {
                case BN_VLIST_LINE_MOVE:
                    /* Move, start line */
                    if (first == 0)
                        glEnd();
                    first = 0;

                    if (true) {
                        mflag = 0;
                        float wireColor[4]={.5,.2,.7,.5};
                        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, wireColor);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);

                        if (false)
                            glDisable(GL_BLEND);
                    }

                    glBegin(GL_LINE_STRIP);
                    glVertex3dv(dpt);
                    break;
                case BN_VLIST_MODEL_MAT:
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    glLoadMatrixf(m);
                    break;
                case BN_VLIST_DISPLAY_MAT:
                    glMatrixMode(GL_PROJECTION);
                    glGetFloatv(GL_PROJECTION_MATRIX, m);
                    glPopMatrix();
                    glLoadIdentity();
                    break;
                case BN_VLIST_POLY_START:
                case BN_VLIST_TRI_START:
                    /* Start poly marker & normal */



                    if (*cmd == BN_VLIST_POLY_START) {
                        if (first == 0)
                            glEnd();

                        glBegin(GL_POLYGON);
                    } else if (first)
                        glBegin(GL_TRIANGLES);

                    /* Set surface normal (vl_pnt points outward) */
                    glNormal3dv(dpt);

                    first = 0;

                    break;
                case BN_VLIST_LINE_DRAW:
                case BN_VLIST_POLY_MOVE:
                case BN_VLIST_POLY_DRAW:
                case BN_VLIST_TRI_MOVE:
                case BN_VLIST_TRI_DRAW:
                    glVertex3dv(dpt);
                    break;
                case BN_VLIST_POLY_END:
                    /* Draw, End Polygon */
                    glEnd();
                    first = 1;
                    break;
                case BN_VLIST_TRI_END:
                    break;
                case BN_VLIST_POLY_VERTNORM:
                case BN_VLIST_TRI_VERTNORM:
                    /* Set per-vertex normal.  Given before vert. */
                    glNormal3dv(dpt);
                    break;
                case BN_VLIST_POINT_DRAW:
                    if (first == 0)
                        glEnd();
                    first = 0;
#if ENABLE_POINT_SMOOTH
                    glEnable(GL_POINT_SMOOTH);
#endif
                    glBegin(GL_POINTS);
                    glVertex3dv(dpt);
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

    char str[1];
    if (first == 0)glEnd();


    glPointSize(originalPointSize);
    glLineWidth(originalLineWidth);


    cout<<onn<<endl;
}

void  GraphicsViewOpenGL::setVlist(struct bn_vlist *vp){


}

void GraphicsViewOpenGL::paintGL() {



    glClear (GL_COLOR_BUFFER_BIT);
    glClear (GL_COLOR_BUFFER_BIT);

    if(vp==0)return;

    glMatrixMode(GL_MODELVIEW);
    int z=-0, y= -0;
    glTranslatef(0,y,z);
    ogl_drawVList();
    glTranslatef(0,-y,-z);

    glColor3f (0.0, 1.0, 0.0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POLYGON_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

    glLineWidth((GLfloat) 3);
    //glColor3f (1.0, 0.0, 0.0);




    glFlush();
}



void GraphicsViewOpenGL::keyPressEvent( QKeyEvent *k )
{


    if(vp==0)return;
    GLfloat matrix[16];
    glGetFloatv( GL_PROJECTION, matrix );

    float DOF[3];
    DOF[0] = 1; // x
    DOF[1] = 1; // y
    DOF[2] = 1; // z
    float dv = sqrt(DOF[0]*DOF[0]+DOF[1]*DOF[1]+DOF[2]*DOF[2]);
    DOF[0]/=dv;
    DOF[1]/=dv;
    DOF[2]/=dv;



    int am =100;
    switch ( k->key() )
    {
        case Qt::Key_Up:

            qDebug() << "UP";

            glMatrixMode(GL_MODELVIEW);
            glTranslatef(0,-am,0);


            break;
        case Qt::Key_Down:
            qDebug() << "DOWN";

            glMatrixMode(GL_MODELVIEW);
            glTranslatef(0,am,0);
            break;
        case Qt::Key_Left:
            qDebug() << "LEFT";
            glMatrixMode(GL_MODELVIEW);
            glTranslatef(am,0,0);
            break;
        case Qt::Key_Right:
            qDebug() << "RIGHT";
            glMatrixMode(GL_MODELVIEW);
            glTranslatef(-am,0,0);
            break;


        case Qt::Key_A:
            qDebug() << "LEFT";
            glMatrixMode(GL_MODELVIEW);
            glTranslatef(DOF[0]*am*0,DOF[1]*am*0,DOF[2]*am);
            break;
        case Qt::Key_Z:
            qDebug() << "RIGHT";
            glMatrixMode(GL_MODELVIEW);
            glTranslatef(-DOF[0]*am*0,-DOF[1]*am*0,-DOF[2]*am);
            break;

        case Qt::Key_Q:
            qDebug() << "RIGHT";
            glMatrixMode(GL_PROJECTION);
            glRotatef(-3, 0,1,0);
            break;

        case Qt::Key_E:
            qDebug() << "RIGHT";
            glMatrixMode(GL_PROJECTION);
            glRotatef(3, 0,1,0);
            break;
        default:
            qDebug() << k->key() << endl;
            break;
    }

    update();

}