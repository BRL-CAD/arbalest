
#ifndef RT3_GRIDRENDERER_H
#define RT3_GRIDRENDERER_H


#include "Renderer.h"
#include "Viewport.h"

class Viewport;

class GridRenderer: public Renderer {

public:
    void render() override;

    GridRenderer(Viewport *display);

private:
    Viewport * display;
};


#endif //RT3_GRIDRENDERER_H
