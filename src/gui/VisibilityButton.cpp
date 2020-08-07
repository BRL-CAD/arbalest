
#include <QtWidgets/QStyledItemDelegate>
#include <QtCore/QtCore>
#include <QApplication>
#include <QMouseEvent>
#include <QRect>
#include <QPainter>
#include <iostream>
#include <include/VisibilityButton.h>


VisibilityButton::VisibilityButton(ObjectTree *objectTree, QObject *parent) : objectTree(objectTree), QStyledItemDelegate(parent) {
    iconFullVisible             = QImage(iconFilePath);
    iconSomeChildrenVisible     = QImage(iconFilePath);
    iconInvisible               = QImage(iconFilePath);

    for (int y = 0; y < iconSomeChildrenVisible.height(); y++) {
        for (int x = 0; x < iconSomeChildrenVisible.width()/2; x++) {
            QColor pixel = iconSomeChildrenVisible.pixelColor(x, y);
            pixel.setBlue(220);
            pixel.setGreen(220);
            pixel.setRed(220);
            iconSomeChildrenVisible.setPixelColor(x, y, pixel);
        }
        for (int x = iconSomeChildrenVisible.width()/2; x < iconSomeChildrenVisible.width(); x++) {
            QColor pixel = iconSomeChildrenVisible.pixelColor(x, y);
            pixel.setBlue(255);
            pixel.setGreen(0);
            pixel.setRed(0);
            iconSomeChildrenVisible.setPixelColor(x, y, pixel);
        }
    }

    for (int y = 0; y < iconFullVisible.height(); y++) {
        for (int x = 0; x < iconFullVisible.width(); x++) {
            QColor pixel = iconFullVisible.pixelColor(x, y);
            pixel.setBlue(255);
            pixel.setGreen(0);
            pixel.setRed(0);
            iconFullVisible.setPixelColor(x, y, pixel);
        }
    }

    for (int y = 0; y < iconInvisible.height(); y++) {
        for (int x = 0; x < iconInvisible.width(); x++) {
            QColor pixel = iconInvisible.pixelColor(x, y);
            pixel.setBlue(170);
            pixel.setGreen(170);
            pixel.setRed(170);
            iconInvisible.setPixelColor(x, y, pixel);
        }
    }
}

QPoint VisibilityButton::iconPos(const QStyleOptionViewItem &option) const {
    return {        option.rect.right() - iconFullVisible.width() - margin-5,
                    option.rect.center().y() - iconFullVisible.height() / 2};
}

void VisibilityButton::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
    if (option.state & QStyle::State_MouseOver) {
        switch (visibilityState){
            case ObjectTree::Invisible:
                painter->drawImage(iconPos(option), iconInvisible);
                break;
            case ObjectTree::SomeChildrenVisible:
                painter->drawImage(iconPos(option), iconSomeChildrenVisible);
                break;
            case ObjectTree::FullyVisible:
                painter->drawImage(iconPos(option), iconFullVisible);
                break;
        }
    }
}

QSize VisibilityButton::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.rwidth() += iconFullVisible.width() + margin * 2;
    size.setHeight(qMax(size.height(), iconFullVisible.height() + margin * 2));
    return size;
}

bool VisibilityButton::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    objectId = index.data(Qt::UserRole).toInt();
    visibilityState = objectTree->getObjectVisibility()[objectId];
    QRect rect = iconFullVisible.rect().translated(iconPos(option));
    // Emit a signal when the icon is clicked
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (rect.contains(mouseEvent->pos())) {
            emit visibilityButtonClicked(objectId);
            visibilityState = objectTree->getObjectVisibility()[objectId];
            return true;
        }
    }
    return false;
}

