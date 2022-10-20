/*         M O V E C A M E R A M O U S E A C T I O N . C P P
 * BRL-CAD
 *
 * Copyright (c) 2022 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file MoveCameraMouseAction.cpp */

#include "DisplayGrid.h"
#include "MoveCameraMouseAction.h"


MoveCameraMouseAction::MoveCameraMouseAction(DisplayGrid* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

MoveCameraMouseAction::~MoveCameraMouseAction() {}

bool MoveCameraMouseAction::eventFilter(QObject* watched, QEvent* event) {
    bool ret = false;

    if (watched == m_watched) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent* moveCameraEvent = static_cast<QMouseEvent *>(event);
            const int x = moveCameraEvent->x();
            const int y = moveCameraEvent->y();
            int globalX = moveCameraEvent->globalX();
            int globalY = moveCameraEvent->globalY();

            bool resetX = false, resetY = false;

            if (prevMouseX != -1 && prevMouseY != -1 && 
                (moveCameraEvent->buttons() & (rotateCameraMouseButton | moveCameraMouseButton))) {
                if (skipNextMouseMoveEvent) {
                    skipNextMouseMoveEvent = false;
                }
                if (moveCameraEvent->buttons() & (rotateCameraMouseButton)) {
                    const bool rotateThirdAxis = QApplication::keyboardModifiers().testFlag(rotateAroundThirdAxisModifier);
                    m_watched->getCamera()->processRotateRequest(x - prevMouseX, y - prevMouseY, rotateThirdAxis);
                }
                if (moveCameraEvent->buttons() & (moveCameraMouseButton)) {
                    m_watched->getCamera()->processMoveRequest(x - prevMouseX, y - prevMouseY);
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
                    prevMouseX = resetX ? m_watched->mapFromGlobal(QPoint(newX, newY)).x() : x;
                    prevMouseY = resetY ? m_watched->mapFromGlobal(QPoint(newX, newY)).y() : y;
                    QCursor::setPos(resetX ? newX : globalX, resetY ? newY : globalY);
                    skipNextMouseMoveEvent = true;
                }
            }

            if (!resetX && !resetY) {
                prevMouseX = x;
                prevMouseY = y;
            }

            ret = true;
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            m_parent->setActiveDisplay(m_watched);

            QMouseEvent* mouseButtonPressEvent = static_cast<QMouseEvent*>(event);
            prevMouseX = mouseButtonPressEvent->x();
            prevMouseY = mouseButtonPressEvent->y();

            ret        = true;
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            prevMouseX = -1;
            prevMouseY = -1;
            ret        = true;

            emit Done(this);
        }
        else if (event->type() == QEvent::Wheel) {
            QWheelEvent* wheelMouseEvent = static_cast<QWheelEvent*>(event);

            if (wheelMouseEvent->phase() == Qt::NoScrollPhase || 
                wheelMouseEvent->phase() == Qt::ScrollUpdate || 
                wheelMouseEvent->phase() == Qt::ScrollMomentum) {
                m_watched->getCamera()->processZoomRequest(wheelMouseEvent->angleDelta().y() / 8);
                m_watched->forceRerenderFrame();
            }

            ret = true;
        }
    }

    return ret;
}
