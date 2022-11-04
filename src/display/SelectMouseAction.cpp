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

SelectMouseAction::~SelectMouseAction() {}

QString SelectMouseAction::getSelected() const {
    return m_selected;
}

class SelectCallback : public BRLCAD::ConstDatabase::HitCallback {
public:
    SelectCallback()
        : BRLCAD::ConstDatabase::HitCallback() {}

    virtual bool operator()(const BRLCAD::ConstDatabase::Hit& hit) throw() {
        m_selected = hit.Name();

        return false;
    }

    const QString& getSelected() const {
        return m_selected;
    }

private:
    QString m_selected;
};

bool SelectMouseAction::eventFilter(QObject* watched, QEvent* event) {
    bool ret = false;
    
    if (watched == m_watched) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

            if (mouseEvent->button() == Qt::LeftButton) {
                const int x = mouseEvent->x();
                const int y = mouseEvent->y();

                int w = m_watched->getDocument()->getRaytraceWidget()->width();
                int h = m_watched->getDocument()->getRaytraceWidget()->height();

                QVector3D &eyePosition = m_watched->getCamera()->getEyePosition();

                QMatrix4x4 m_transformation;
                QVector3D directionStart = m_transformation.map(eyePosition);
                QVector3D directionEnd = m_transformation.map(QVector3D(0., 0., 0.));
                QVector3D direction = directionEnd - directionStart;
                direction.normalize();

                QVector3D        imagePoint(x, h - y - 1., 0.);
                QVector3D        modelPoint = m_transformation.map(imagePoint);
                SelectCallback callback;
                BRLCAD::Ray3D    ray;

                ray.origin.coordinates[0] = modelPoint.x();
                ray.origin.coordinates[1] = modelPoint.y();
                ray.origin.coordinates[2] = modelPoint.z();
                ray.direction.coordinates[0] = direction.x();
                ray.direction.coordinates[1] = direction.y();
                ray.direction.coordinates[2] = direction.z();

                m_watched->getDocument()->getDatabase()->ShootRay(ray, callback, BRLCAD::ConstDatabase::StopAfterFirstHit);
                m_selected = callback.getSelected();

                if (!m_selected.isEmpty()) {
                    emit Done(this);
                }

                ret = true;
            }
            else {
                ret = false;

            }
        }
    }

    return ret;
}
