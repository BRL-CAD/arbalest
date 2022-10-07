#include "PressMouseAction.h"

PressMouseAction::PressMouseAction(QObject* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

PressMouseAction::~PressMouseAction() {}

bool PressMouseAction::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_watched) {
        if (event->type() == QEvent::MouseButtonPress) {
            m_watched->getDocument()->getDisplayGrid()->setActiveDisplay(m_watched);
            
            QMouseEvent* moveCameraEvent = static_cast<QMouseEvent*>(event);
            m_watched->setPrevMouseX(moveCameraEvent->x());
            m_watched->setPrevMouseY(moveCameraEvent->y());
        }
    }

    return false;
}