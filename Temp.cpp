


switch (*cmd) {
case BN_VLIST_LINE_MOVE:
    /* Move, start line */
    if (first == 0)
    glEnd();
    first = 0;

    if (dmp->dm_light && mflag) {
    mflag = 0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, wireColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);

    if (dmp->dm_transparency)
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
    glScaled(2. * 3.78 / dmp->dm_width,
             2. * 3.78 / dmp->dm_height,
             1.);
    break;
case BN_VLIST_POLY_START:
case BN_VLIST_TRI_START:
    /* Start poly marker & normal */

    if (dmp->dm_light && mflag) {
    mflag = 0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);

    switch (dmp->dm_light) {
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

    if (dmp->dm_transparency)
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
    GLfloat lineWidth = (GLfloat)(*pt)[0];
    if (lineWidth > 0.0) {
    glLineWidth(lineWidth);
    }
    break;
}
case BN_VLIST_POINT_SIZE: {
    GLfloat pointSize = (GLfloat)(*pt)[0];
    if (pointSize > 0.0) {
    glPointSize(pointSize);
    }
    break;
}
}