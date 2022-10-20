/*                      M O U S E A C T I O N . H
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
/** @file MouseAction.h */

#ifndef MOUSEACTION_H
#define MOUSEACTION_H

#include <QObject>

class Display;
class DisplayGrid;


class MouseAction : public QObject
{
    Q_OBJECT
public:
    virtual ~MouseAction();

protected:
    DisplayGrid* m_parent;
    Display*     m_watched;

    explicit MouseAction(DisplayGrid* parent = nullptr, Display* watched = nullptr);

signals:
    void Done(MouseAction* myself);

private slots:
    void WatchedDestroyed(QObject* watched);
};

#endif // MOUSEACTION_H
