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

bool SelectMouseAction::eventFilter(QObject* watched, QEvent* event) {
    bool ret = false;
    
    if (watched == m_watched) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

            if (mouseEvent->button() == Qt::LeftButton) {
                const int x = mouseEvent->x();
                const int y = mouseEvent->y();

                // Shoot a ray

                // Check if a ray intersects or hit

                // Dynamically cast BRLCAD::ConstDatabase to BRLCAD::ConstDatabase::Hit and return region Name

                emit Done(this);

                ret = true;
            }

            ret = false;
        }
    }

    return ret;
}
