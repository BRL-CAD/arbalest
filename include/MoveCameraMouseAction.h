/*           M O V E C A M E R A M O U S E A C T I O N . H
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
/** @file MoveCameraMouseAction.h */

#ifndef MOVECAMERAMOUSEACTION_H
#define MOVECAMERAMOUSEACTION_H

#include "MouseAction.h"


class MoveCameraMouseAction : public MouseAction
{
public:
    explicit MoveCameraMouseAction(DisplayGrid* parent = nullptr, Display* watched = nullptr);
    virtual ~MoveCameraMouseAction();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    int prevMouseX = -1;
    int prevMouseY = -1;
    bool skipNextMouseMoveEvent = false;

    Qt::MouseButton rotateCameraMouseButton = Qt::LeftButton;
    Qt::MouseButton moveCameraMouseButton = Qt::RightButton;
    Qt::KeyboardModifier rotateAroundThirdAxisModifier = Qt::ShiftModifier;
};

#endif // MOVECAMERAMOUSEACTION_H
