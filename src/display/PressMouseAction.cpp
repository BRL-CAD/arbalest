#include "PressMouseAction.h"

PressMouseAction::PressMouseAction(QObject* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

PressMouseAction::~PressMouseAction() {}

bool PressMouseAction::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_watched) {
        if (event->type() == QEvent::MouseButtonPress) {
            document->getDisplayGrid()->setActiveDisplay(this);
            QMouseEvent* moveCameraEvent = static_cast<QMouseEvent*>(event);
            prevMouseX = moveCameraEvent->x();
            prevMouseY = moveCameraEvent->y();
        }
    }
}