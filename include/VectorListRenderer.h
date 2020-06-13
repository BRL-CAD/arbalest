//
// Created by Sadeep on 12-Jun.
//

#ifndef RT3_VECTORLISTRENDERER_H
#define RT3_VECTORLISTRENDERER_H


#include <brlcad/VectorList.h>

class VectorListRenderer : public BRLCAD::VectorList::ElementCallback {
public:
    void render(BRLCAD::VectorList * vectorList,int w, int h) ;
    bool operator()(BRLCAD::VectorList::Element* element) override ;
private:
    int dm_light = 1;
    bool dm_transparency = 0;
    int	first;
    int mflag = 1;
    GLfloat originalPointSize, originalLineWidth;
    GLdouble m[16];
    GLdouble mt[16];
    GLdouble tlate[3];
    const  float black[4] = {0.0, 0.0, 0.0, 0.0};
    float wireColor[4] = {.9,.1,.1,1};
    float diffuseColor[4];
    float  ambientColor[4];
    float  specularColor[4];
    float  backDiffuseColorDark[4];
    float backDiffuseColorLight[4];
    int w,h;
};


#endif //RT3_VECTORLISTRENDERER_H
