#include "MoveCameraMouseAction.h"


MoveCameraMouseAction::MoveCameraMouseAction(QObject* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

MoveCameraMouseAction::~MoveCameraMouseAction() {}

bool MoveCameraMouseAction::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_watched) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent* moveCameraEvent = static_cast<QMouseEvent *>(event);
            const int x = moveCameraEvent->x();
            const int y = moveCameraEvent->y();
            int globalX = moveCameraEvent->globalX();
            int globalY = moveCameraEvent->globalY();

            bool resetX = false, resetY = false;

            if (m_watched->getPrevMouseX() != -1 && m_watched->getPrevMouseY() != -1 && 
                (moveCameraEvent->buttons() & (m_watched->getRotateCamera() | m_watched->getMoveCamera()))) {
                if (m_watched->getSkipNextMouseMoveEvent()) {
                    m_watched->setSkipNextMouseMoveEvent(false);
                }
                if (moveCameraEvent->buttons() & (m_watched->getRotateCamera())) {
                    const bool rotateThirdAxis = QApplication::keyboardModifiers().testFlag(m_watched->getRotateAroundThirdAxisModifier());
                    m_watched->getCamera()->processRotateRequest(x - m_watched->getPrevMouseX(), y - m_watched->getPrevMouseY(), rotateThirdAxis);
                }
                if (moveCameraEvent->buttons() & (m_watched->getMoveCamera())) {
                    m_watched->getCamera()->processMoveRequest(x - m_watched->getPrevMouseX(), y - m_watched->getPrevMouseY());
                }

                m_watched->forceRerenderFrame();

                const QPoint topLeft = m_watched->mapToGlobal(QPoint(0, 0));
                const QPoint bottomRight = m_watched->mapToGlobal(QPoint(m_watched->size().width(), m_watched->size().height()));

                int newX = -1;
                int newY = -1;

                if (globalX <= topLeft.x()) {
                    newX = bottomRight.x() - 1;
                    resetX = true;
                }
                if (globalX >= bottomRight.x()) {
                    newX = topLeft.x() + 1;
                    resetX = true;
                }
                if (globalY <= topLeft.y()) {
                    newY = bottomRight.y() - 1;
                    resetY = true;
                }
                if (globalY >= bottomRight.y()) {
                    newY = topLeft.y() + 1;
                    resetY = true;
                }

                if (resetX || resetY) {
                    m_watched->setPrevMouseX(resetX ? m_watched->mapFromGlobal(QPoint(newX, newY)).x() : x);
                    m_watched->setPrevMouseY(resetY ? m_watched->mapFromGlobal(QPoint(newX, newY)).y() : y);
                    QCursor::setPos(resetX ? newX : globalX, resetY ? newY : globalY);
                    m_watched->setSkipNextMouseMoveEvent(true);
                }
            }

            if (!resetX && !resetY) {
                m_watched->setPrevMouseX(x);
                m_watched->setPrevMouseY(y);
            }
        }
    }

    return false;
}