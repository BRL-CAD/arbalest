
#ifndef RT3_OBJECTTREEROWBUTTONS_H
#define RT3_OBJECTTREEROWBUTTONS_H

#include <QtWidgets/QStyledItemDelegate>
#include <QtCore/QtCore>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <iostream>
#include "ObjectTree.h"
#include "Document.h"

class ObjectTreeRowButtons : public QStyledItemDelegate {
Q_OBJECT
public:
    explicit ObjectTreeRowButtons(ObjectTree *objectTree, QObject *parent = nullptr);
    QPoint visibilityIconPosition(const QStyleOptionViewItem &option) const;
    QPoint centerIconPosition(const QStyleOptionViewItem &option) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QImage iconFullVisible;
    QImage iconSomeChildrenVisible;
    QImage iconInvisible;

    QImage iconCenter;

    size_t objectId = 0;
    ObjectTree *objectTree;
    ObjectTreeItem::VisibilityState visibilityState = ObjectTreeItem::Invisible;
    bool alive = false;
    static const int margin = 0;
    const char *const visibilityIconFilePath = ":/icons/baseline_visibility_black_18dp.png";
    const char *const centerIconFilePath = ":/icons/baseline_center_focus_strong_black_18dp.png";
    Q_DISABLE_COPY(ObjectTreeRowButtons)

signals:
    void visibilityButtonClicked(size_t objectId);
    void centerButtonClicked(size_t objectId);

};

#endif //RT3_OBJECTTREEROWBUTTONS_H
