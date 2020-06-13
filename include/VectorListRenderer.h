//
// Created by Sadeep on 12-Jun.
//

#ifndef RT3_VECTORLISTRENDERER_H
#define RT3_VECTORLISTRENDERER_H


#include <brlcad/VectorList.h>

class VectorListRenderer {
public:
    static void render(BRLCAD::VectorList * vectorList,Display* display);

private:
    class Callback : public BRLCAD::VectorList::ElementCallback {
        bool operator()(BRLCAD::VectorList::Element* element) override ;
    };
};


#endif //RT3_VECTORLISTRENDERER_H
