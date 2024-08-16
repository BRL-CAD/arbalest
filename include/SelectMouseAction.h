/*           S E L E C T M O U S E A C T I O N . H
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
 /** @file SelectMouseAction.h */

#ifndef SELECTMOUSEACTION_H
#define SELECTMOUSEACTION_H

#include "MouseAction.h"

class SelectMouseAction : public MouseAction
{
public:
    explicit SelectMouseAction(DisplayGrid* parent = nullptr, Display* watched = nullptr);
    virtual ~SelectMouseAction();
    void Deselect(Display* m_watched);

    QString getSelected() const;

protected:
    QString m_selected;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // SELECTMOUSEACTION_H
