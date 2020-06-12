//
// Created by Sadeep on 12-Jun.
//
#ifdef _WIN32

#include <windows.h>

#endif

#include <gl/GL.h>
#include "VectorListRenderer.h"

void VectorListRenderer::render(BRLCAD::VectorList *vectorList) {
    Callback elementOpenGL;
    vectorList->Iterate(elementOpenGL);
}


bool VectorListRenderer::Callback::operator()(BRLCAD::VectorList::Element *element) {
    if (!element) return true;

    switch (element->Type()) {

        case BRLCAD::VectorList::Element::LineMove: {
            break;
        }
        case BRLCAD::VectorList::Element::PointDraw: {
            break;
        }
        case BRLCAD::VectorList::Element::PointSize: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PointSize *> (element);
            auto pointSize = (GLfloat) e->Size();
            if (pointSize > 0.0) {
                glPointSize(pointSize);
            }
            break;
        }
        case BRLCAD::VectorList::Element::LineDraw: {
            break;
        }
        case BRLCAD::VectorList::Element::LineWidth: {
            auto *e = dynamic_cast<BRLCAD::VectorList::LineWidth *> (element);
            auto lineWidth = (GLfloat) e->Width();
            if (lineWidth > 0.0) {
                glLineWidth(lineWidth);
            }
            break;
        }
        case BRLCAD::VectorList::Element::TriangleStart: {
            break;
        }
        case BRLCAD::VectorList::Element::TriangleMove: {
            break;
        }
        case BRLCAD::VectorList::Element::TriangleDraw: {
            break;
        }
        case BRLCAD::VectorList::Element::TriangleEnd: {
            break;
        }
        case BRLCAD::VectorList::Element::TriangleVertexNormal: {
            break;
        }
        case BRLCAD::VectorList::Element::PolygonStart: {
            break;
        }
        case BRLCAD::VectorList::Element::PolygonMove: {
            break;
        }
        case BRLCAD::VectorList::Element::PolygonDraw: {
            break;
        }
        case BRLCAD::VectorList::Element::PolygonEnd: {
            break;
        }
        case BRLCAD::VectorList::Element::PolygonVertexNormal: {
            break;
        }
        case BRLCAD::VectorList::Element::DisplaySpace: {
            break;
        }
        case BRLCAD::VectorList::Element::ModelSpace: {
            break;
        }
    }
    return true;
}
