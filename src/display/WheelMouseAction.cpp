#include "WheelMouseAction.h"

WheelMouseAction::WheelMouseAction(QObject* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

WheelMouseAction::~WheelMouseAction() {}

bool WheelMouseAction::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_watched) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent* wheelMouseEvent = static_cast<QWheelEvent*>(event);

            if (wheelMouseEvent->phase() == Qt::NoScrollPhase || wheelMouseEvent->phase() == Qt::ScrollUpdate || wheelMouseEvent->phase() == Qt::ScrollMomentum) {
                m_watched->getCamera()->processZoomRequest(wheelMouseEvent->angleDelta().y() / 8);
                m_watched->forceRerenderFrame();
            }
        }
    }

    return false;
}
