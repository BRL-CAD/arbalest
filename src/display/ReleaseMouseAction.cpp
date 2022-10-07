#include "ReleaseMouseAction.h"

ReleaseMouseAction::ReleaseMouseAction(QObject* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

ReleaseMouseAction::~ReleaseMouseAction() {}

bool ReleaseMouseAction::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_watched) {
        if (event->type() == QEvent::MouseButtonRelease) {
            m_watched->setPrevMouseX(-1);
            m_watched->setPrevMouseY(-1);
        }
    }

    return false;
}