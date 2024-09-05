/*           S E L E C T M O U S E A C T I O N . C P P
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
 /** @file SelectMouseAction.cpp */

#include "SelectMouseAction.h"
#include "DisplayGrid.h"


SelectMouseAction::SelectMouseAction(DisplayGrid* parent, Display* watched)
    : MouseAction(parent, watched) {
    m_watched->installEventFilter(this);
}

SelectMouseAction::~SelectMouseAction() {
    DisplayManager* displayManager = m_watched->getDisplayManager();
    if (displayManager) {
        displayManager->clearSuffix();
        m_watched->forceRerenderFrame();
    }
}

QString SelectMouseAction::getSelected() const {
    return m_selected;
}

bool SelectMouseAction::eventFilter(QObject* watched, QEvent* event) {
    bool ret = false;
    
    if (watched == m_watched) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

            if (mouseEvent->button() == Qt::LeftButton) {
                QMatrix4x4 transformation;
                transformation.translate(m_watched->getCamera()->getEyePosition().x(),
                    m_watched->getCamera()->getEyePosition().y(),
                    m_watched->getCamera()->getEyePosition().z());
                transformation.rotate(-m_watched->getCamera()->getAnglesAroundAxes().y(), 0., 1., 0.);
                transformation.rotate(-m_watched->getCamera()->getAnglesAroundAxes().z(), 0., 0., 1.);
                transformation.rotate(-m_watched->getCamera()->getAnglesAroundAxes().x(), 1., 0., 0.);
                transformation.translate(0, 0, 10000);
                transformation.scale(m_watched->getCamera()->getVerticalSpan() / m_watched->getH());
                transformation.translate(-m_watched->getW() / 2., -m_watched->getH() / 2.);
                
                QVector3D directionStart = transformation.map(QVector3D(0., 0., 1.));
                QVector3D directionEnd = transformation.map(QVector3D(0., 0., 0.));
                QVector3D direction = directionEnd - directionStart;
                direction.normalize();

                QVector3D imagePoint(mouseEvent->x(), m_watched->getH() - mouseEvent->y() - 1., 0.);
                QVector3D modelPoint = transformation.map(imagePoint);
                
                BRLCAD::Ray3D    ray;
                ray.origin.coordinates[0] = modelPoint.x();
                ray.origin.coordinates[1] = modelPoint.y();
                ray.origin.coordinates[2] = modelPoint.z();
                ray.direction.coordinates[0] = direction.x();
                ray.direction.coordinates[1] = direction.y();
                ray.direction.coordinates[2] = direction.z();

                m_selected.clear();
                m_watched->getDocument()->getDatabase()->ShootRay(ray, [this](const BRLCAD::ConstDatabase::Hit& hit){m_selected = hit.Name(); return false;}, BRLCAD::ConstDatabase::StopAfterFirstHit);

                if (!m_selected.isEmpty()) {
                    emit Done(this);

                    BRLCAD::VectorList vectorList;
                    const QString objectFullPath = m_selected;
                    m_watched->getDocument()->getDatabase()->Plot(objectFullPath.toUtf8(), vectorList);
                    m_watched->getDisplayManager()->setSuffix(vectorList);
                    m_watched->getDisplayManager()->drawSuffix();
                    m_watched->forceRerenderFrame();
                }
            }

                ret = true;
        }
            else {
                ret = false;

            }
    }

    return ret;
}
