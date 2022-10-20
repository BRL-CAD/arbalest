/*                   M O U S E A C T I O N . C P P
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
/** @file MouseAction.cpp */

#include "DisplayGrid.h"
#include "MouseAction.h"


MouseAction::MouseAction(DisplayGrid* parent, Display* watched)
    : QObject(parent), m_parent(parent), m_watched(watched) {
    connect(m_watched, &Display::destroyed,
            this,      &MouseAction::WatchedDestroyed);
}

MouseAction::~MouseAction() {}

void MouseAction::WatchedDestroyed(QObject* watched) {
    if (watched == m_watched)
        m_watched = nullptr;
}
