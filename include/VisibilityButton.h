
#ifndef RT3_VISIBILITYBUTTON_H
#define RT3_VISIBILITYBUTTON_H

#include <QtWidgets/QStyledItemDelegate>
#include <QtCore/QtCore>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <iostream>
#include "ObjectTree.h"
#include "Document.h"

class VisibilityButton : public QStyledItemDelegate {
Q_OBJECT
public:
    explicit VisibilityButton(ObjectTree *objectTree, QObject *parent = nullptr);
    QPoint iconPos(const QStyleOptionViewItem &option) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QImage iconFullVisible;
    QImage iconSomeChildrenVisible;
    QImage iconInvisible;

    int objectId = 0;
    ObjectTree *objectTree;

    ObjectTree::VisibilityState visibilityState = ObjectTree::Invisible;
    static const int margin = 0; // pixels to keep around the icon
    const char *const iconFilePath = ":/icons/icons8-eye-20.png";
    Q_DISABLE_COPY(VisibilityButton)

signals:
    void visibilityButtonClicked(int objectId);

};

#endif //RT3_VISIBILITYBUTTON_H
