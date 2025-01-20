
#ifndef RT3_GRIDRENDERER_H
#define RT3_GRIDRENDERER_H


#include "Renderer.h"
#include "ArbDisplay.h"

class ArbDisplay;

class GridRenderer: public Renderer {

public:
    void render() override;

    GridRenderer(ArbDisplay *display);

private:
    ArbDisplay * display;
};


#endif //RT3_GRIDRENDERER_H
