//
// Created by Sadeep on 12-Jun.
//

#ifndef RT3_VLISTRENDERER_H
#define RT3_VLISTRENDERER_H


#include <brlcad/MemoryDatabase.h>

class VListRenderer{
public:
    void render(bn_vlist *vp,int w, int h) ;
private:
    int dm_light = 1;
    bool dm_transparency = 0;
    float wireColor[4] = {.9,.1,.1,1};
    float diffuseColor[4];
    float  ambientColor[4];
    float  specularColor[4];
    float  backDiffuseColorDark[4];
    float backDiffuseColorLight[4];
    int w,h;
};


#endif //RT3_VLISTRENDERER_H
