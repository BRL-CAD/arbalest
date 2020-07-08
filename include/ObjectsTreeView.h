/*                      O B J E C T S T R E E V I E W . H
 * BRL-CAD
 *
 * Copyright (c) 2018 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/** @file ObjectsTreeView.h
 *
 *  taken from RT3/QtGUI:
 *      declaration of the objects' tree visualization
 */

#ifndef OBJECTSTREEVIEW_H
#define OBJECTSTREEVIEW_H

#include <QTreeView>
#include <QStandardItemModel>

#include <brlcad/ConstDatabase.h>


class ObjectsTreeView : public QTreeView {
    Q_OBJECT
public:
    ObjectsTreeView(BRLCAD::ConstDatabase& database,
                    QWidget*               parent = 0);

    void Rebuild(void);

signals:
    void SelectionChanged(void);

private:
    BRLCAD::ConstDatabase& m_database;
    QStandardItemModel*    m_objectsTree;

private slots:
    void Activated(const QItemSelection &, const QItemSelection &);
};


#endif // OBJECTSTREEVIEW_H
