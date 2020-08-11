
#ifndef RT3_GRIDRENDERER_H
#define RT3_GRIDRENDERER_H


#include "Renderer.h"
#include "Display.h"

class Display;

class GridRenderer: public Renderer {

public:
    void render() override;

    GridRenderer(Display *display);

private:
    Display * display;
};


#endif //RT3_GRIDRENDERER_H
